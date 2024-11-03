#include "qemu/osdep.h"

#include "block/block-io.h"
#include "block/block_int-io.h"
#include "block/snapshot.h"
#include "io/channel-buffer.h"
#include "io/channel-file.h"
#include "migration/global_state.h"
#include "migration/migration.h"
#include "migration/qemu-file.h"
#include "migration/savevm.h"
#include "monitor/hmp.h"
#include "monitor/monitor.h"
#include "qapi/error.h"
#include "qapi/qapi-builtin-visit.h"
#include "qapi/qapi-commands-block.h"
#include "qapi/qmp/qdict.h"
#include "qapi/qmp/qerror.h"
#include "qapi/util.h"
#include "qemu/cutils.h"
#include "qemu/log.h"
#include "qemu/main-loop.h"
#include "qemu/typedefs.h"
#include "sysemu/replay.h"
#include "sysemu/runstate.h"

#include "snapvm-external.h"

#include <gio/gio.h> /* Required for GIO error codes */
#include <zstd.h>

typedef struct {
        gchar *filepath;
        gint compression_level;
        GError *error;
} CompressionData;

static gboolean compress_chunk(ZSTD_CStream *cstream, GInputStream *input,
                               GOutputStream *output, void *buff_in,
                               void *buff_out, size_t buff_in_size,
                               size_t buff_out_size, ZSTD_EndDirective mode,
                               GError **error)
{
        gsize bytes_read;
        gboolean success = g_input_stream_read_all(input, buff_in, buff_in_size,
                                                   &bytes_read, NULL, error);
        if (!success) {
                return FALSE;
        }

        ZSTD_inBuffer input_buf = {buff_in, bytes_read, 0};
        while (input_buf.pos < input_buf.size) {
                ZSTD_outBuffer output_buf = {buff_out, buff_out_size, 0};
                size_t ret = ZSTD_compressStream2(cstream, &output_buf,
                                                  &input_buf, mode);
                if (ZSTD_isError(ret)) {
                        g_set_error(error, G_IO_ERROR, G_IO_ERROR_FAILED,
                                    "ZSTD compression error: %s",
                                    ZSTD_getErrorName(ret));
                        return FALSE;
                }

                if (output_buf.pos > 0) {
                        if (!g_output_stream_write_all(output, buff_out,
                                                       output_buf.pos, NULL,
                                                       NULL, error)) {
                                return FALSE;
                        }
                }
        }

        return TRUE;
}

static gboolean flush_remaining_data(ZSTD_CStream *cstream,
                                     GOutputStream *output, void *buff_out,
                                     size_t buff_out_size, GError **error)
{
        size_t remaining;
        do {
                ZSTD_outBuffer output_buf = {buff_out, buff_out_size, 0};
                remaining = ZSTD_endStream(cstream, &output_buf);
                if (ZSTD_isError(remaining)) {
                        g_set_error(error, G_IO_ERROR, G_IO_ERROR_FAILED,
                                    "ZSTD flush error: %s",
                                    ZSTD_getErrorName(remaining));
                        return FALSE;
                }

                if (output_buf.pos > 0) {
                        if (!g_output_stream_write_all(output, buff_out,
                                                       output_buf.pos, NULL,
                                                       NULL, error)) {
                                return FALSE;
                        }
                }
        } while (remaining > 0);

        return TRUE;
}

static gpointer compress_state_file(gpointer user_data)
{
        CompressionData *data = (CompressionData *)user_data;
        GError *error = NULL;
        void *buff_in = NULL;
        void *buff_out = NULL;
        GFile *input_file = NULL;
        GFile *output_file = NULL;
        GInputStream *input_stream = NULL;
        GOutputStream *output_stream = NULL;
        ZSTD_CStream *cstream = NULL;
        /* Create output filename */
        gchar* compressed_file = g_strdup_printf("%s.zst", data->filepath);

        /* Allocate compression buffers */
        size_t const buff_in_size = ZSTD_CStreamInSize();
        size_t const buff_out_size = ZSTD_CStreamOutSize();
        buff_in = g_malloc(buff_in_size);
        buff_out = g_malloc(buff_out_size);

        if (!buff_in || !buff_out) {
                g_set_error(&error, G_IO_ERROR, G_IO_ERROR_FAILED,
                            "Failed to allocate compression buffers");
                goto cleanup;
        }


        /* Setup input and output files */
        input_file = g_file_new_for_path(data->filepath);
        output_file = g_file_new_for_path(compressed_file);
        input_stream = G_INPUT_STREAM(g_file_read(input_file, NULL, &error));
        if (!input_stream) {
                goto cleanup;
        }

        output_stream = G_OUTPUT_STREAM(g_file_replace(
            output_file, NULL, FALSE, G_FILE_CREATE_PRIVATE, NULL, &error));
        if (!output_stream) {
                goto cleanup;
        }

        /* Initialize compression */
        cstream = ZSTD_createCStream();
        if (!cstream) {
                g_set_error(&error, G_IO_ERROR, G_IO_ERROR_FAILED,
                            "Failed to initialize ZSTD compression");
                goto cleanup;
        }

        ZSTD_CCtx_setParameter(cstream, ZSTD_c_compressionLevel,
                               data->compression_level);

        /* Compress file content */
        while (!g_input_stream_is_closed(input_stream)) {
                if (!compress_chunk(cstream, input_stream, output_stream,
                                    buff_in, buff_out, buff_in_size,
                                    buff_out_size, ZSTD_e_continue, &error)) {
                        goto cleanup;
                }
        }

        /* Flush remaining data */
        if (!flush_remaining_data(cstream, output_stream, buff_out,
                                  buff_out_size, &error)) {
                goto cleanup;
        }

cleanup:
        if (cstream) {
                ZSTD_freeCStream(cstream);
        }
        g_clear_object(&input_stream);
        g_clear_object(&output_stream);
        g_clear_object(&input_file);
        g_clear_object(&output_file);
        g_free(buff_in);
        g_free(buff_out);
        g_free(compressed_file);

        if (error) {
                data->error = error;
                g_warning("Compression failed: %s", error->message);
        }

        // TODO - xusine
        // If this work well here can be added a line that delete the original dump
        // g_unlink(data->filepath) == 0 || g_error("Failed to delete file: %s", data->filepath);

        return NULL;
}

static void start_compression_thread(const char *filepath)
{
        CompressionData *data = g_new0(CompressionData, 1);
        data->filepath = g_strdup(filepath);
        data->compression_level = 3; /* Default compression level */
        data->error = NULL;

        GThread *thread =
            g_thread_new("state-compression", compress_state_file, data);

        /* We don't need the thread reference since we're not joining */
        g_thread_unref(thread);
}

bool save_snapshot_external(const char *name, bool overwrite,
                            const char *vmstate, bool has_devices,
                            strList *devices, Error **errp)
{
        QEMUSnapshotInfo sn1, *sn = &sn1;
        QEMUFile *f;
        int ret = -1, ret2;
        int saved_vm_running;

        GLOBAL_STATE_CODE();

        if (migration_is_blocked(errp)) {
                return false;
        }

        if (!replay_can_snapshot()) {
                error_setg(errp, "Record/replay does not allow making snapshot "
                                 "right now. Try once more later.");
                return false;
        }

        if (!bdrv_all_can_snapshot(has_devices, devices, errp)) {
                return false;
        }

        /* Delete old snapshots of the same name */
        if (name && overwrite) {
                if (bdrv_all_delete_snapshot(name, has_devices, devices, errp) <
                    0) {
                        return false;
                }
        }

        saved_vm_running = runstate_is_running();
        global_state_store();
        vm_stop(RUN_STATE_SAVE_VM);
        bdrv_drain_all_begin();

        memset(sn, 0, sizeof(*sn));
        /* fill auxiliary fields */
        g_autoptr(GDateTime) now = g_date_time_new_now_local();
        sn->date_sec = g_date_time_to_unix(now);
        sn->date_nsec = g_date_time_get_microsecond(now) * 1000;
        sn->vm_clock_nsec = qemu_clock_get_ns(QEMU_CLOCK_VIRTUAL);
        if (replay_mode != REPLAY_MODE_NONE) {
                sn->icount = replay_get_current_icount();
        } else {
                sn->icount = -1ULL;
        }

        if (name) {
                pstrcpy(sn->name, sizeof(sn->name), name);
        } else {
                g_autofree char *autoname =
                    g_date_time_format(now, "vm-%Y_%m_%d-%H%M_%S");
                pstrcpy(sn->name, sizeof(sn->name), autoname);
        }

        /* save the VM state to the specified output file */
        g_autofree char *output_state_file =
            g_build_filename(qemu_snapvm_state.path, sn->name, NULL);

        QIOChannelFile *ioc;
        ioc = qio_channel_file_new_path(
            output_state_file, O_WRONLY | O_CREAT | O_TRUNC, 0660, errp);
        f = qemu_file_new_output(QIO_CHANNEL(ioc));

        if (!f) {
                error_setg(errp, "Could not open output file %s",
                           output_state_file);
                goto the_end;
        }

        ret = qemu_savevm_state(f, errp);
        ret2 = qemu_fclose(f);

        /**
         * TODO: Create new thread that will compress the outputed file using
         * libzstd
         * */

        if (ret < 0) {
                goto the_end;
        }
        if (ret2 < 0) {
                ret = ret2;
                goto the_end;
        }

        ret = 0;
        start_compression_thread(output_state_file);

the_end:
        bdrv_drain_all_end();
        if (saved_vm_running) {
                vm_start();
        }
        return ret == 0;
}
