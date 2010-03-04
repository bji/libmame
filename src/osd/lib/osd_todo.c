osd_ticks_t osd_ticks(void);


osd_ticks_t osd_ticks_per_second(void);

void osd_sleep(osd_ticks_t duration);


osd_lock *osd_lock_alloc(void);


void osd_lock_acquire(osd_lock *lock);


int osd_lock_try(osd_lock *lock);


void osd_lock_release(osd_lock *lock);


void osd_lock_free(osd_lock *lock);

osd_work_queue *osd_work_queue_alloc(int flags);


int osd_work_queue_items(osd_work_queue *queue);


int osd_work_queue_wait(osd_work_queue *queue, osd_ticks_t timeout);

void osd_work_queue_free(osd_work_queue *queue);

osd_work_item *osd_work_item_queue_multiple(osd_work_queue *queue, osd_work_callback callback, INT32 numitems, void *parambase, INT32 paramstep, UINT32 flags);


int osd_work_item_wait(osd_work_item *item, osd_ticks_t timeout);


void *osd_work_item_result(osd_work_item *item);


void osd_work_item_release(osd_work_item *item);


void *osd_malloc(size_t size);

void osd_free(void *ptr);

void *osd_alloc_executable(size_t size);

void osd_free_executable(void *ptr, size_t size);


void osd_break_into_debugger(const char *message);


char *osd_get_clipboard_text(void);


#ifdef MESS

osd_directory_entry *osd_stat(const char *path);

file_error osd_get_full_path(char **dst, const char *path);

const char *osd_get_volume_name(int idx);

#endif

/**
 * Implementation of osd functions from osdefault.h
 **/

void osd_init(running_machine *machine);

void osd_wait_for_debugger(running_device *device, int firststop);


void osd_update(running_machine *machine, int skip_redraw);



void osd_update_audio_stream(running_machine *machine, INT16 *buffer, int samples_this_frame);

void osd_set_mastervolume(int attenuation);

void osd_customize_input_type_list(input_type_desc *typelist);

