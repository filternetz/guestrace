#ifndef GUESTRACE_H
#define GUESTRACE_H

#include <libvmi/libvmi.h>
#include <libvmi/events.h>
#include <glib.h>
#include <libxl.h>
#include <xenctrl.h>

/**
 * GTLoop
 *
 * The `GTLoop` struct is an opaque data type
 * representing the main event loop of a guestrace application.
 */
typedef struct _GTLoop GTLoop;

typedef addr_t gt_tid_t;

/**
 * GTSyscallFunc:
 * @vmi: the libvmi instance which abstracts the guest.
 * @event: the event which abstracts the system call which caused the guestrace
 * event loop to invoke this function.
 * @pid: the ID of the process running when the event occurred.
 * @tid: the unique ID of the thread running within the current process.
 * @user_data: optional data which might have been passed to the
 * corresponding gt_loop_set_cb(); if set, the guestrace event loop will pass it
 * here.
 * 
 * Specifies one of the two types of functions passed to gt_loop_set_cb().
 * The guestrace event loop invokes this callback each time a program running
 * on the guest invokes the corresponding system call. Implementations can
 * optionally return a pointer which the guestrace event loop will later pass
 * to the corresponding #GTSysretFunc after the system call returns.
 */
typedef void *(*GTSyscallFunc) (vmi_instance_t vmi,
                                vmi_event_t *event,
                                vmi_pid_t pid,
                                gt_tid_t tid,
                                void *user_data);

/**
 * GTSysretFunc:
 * @vmi: the libvmi instance which abstracts the guest.
 * @event: the event which abstracts the system return which caused the guestrace event loop to invoke this function.
 * @pid: the ID of the process running when the event occurred.
 * @tid: the unique ID of the thread running within the current process.
 * @user_data: the return value from #GTSyscallFunc which the guestrace event loop passes to #GTSysretFunc.
 * 
 * Specifies one of the two types of functions passed to gt_loop_set_cb().
 * The guestrace event loop invokes this callback each time a system call on
 * the guest returns control to a program. It is the responsibility of each
 * #GTSysretFunc implementation to free @user_data if the corresponding
 * #GTSyscallFunc returned a pointer to a dynamically-allocated object.
 */
typedef void (*GTSysretFunc) (vmi_instance_t vmi,
                              vmi_event_t *event,
                              vmi_pid_t pid,
                              gt_tid_t tid,
                              void *user_data);

/**
 * GTSyscallCallbacks:
 * @name: the name of the kernel function to instrument.
 * @syscall_cb: the #GTSyscallFunc which the guestrace event loop will invoke upon @name being called.
 * @sysret_cb: the #GTSysretFunc which the guestrace event loop will invoke when @name returns.
 *
 * Full callback definition for use with gt_loop_set_cbs().
 */
typedef struct GTSyscallCallback {
        /* <private> */
        char         *name;
        GTSyscallFunc syscall_cb;
        GTSysretFunc  sysret_cb;
        void         *user_data;
} GTSyscallCallback;

/**
 * GTOSType:
 * @GT_OS_UNKNOWN: an unknown operating system.
 * @GT_OS_LINUX: a Linux operating system.
 * @GT_OS_WINDOWS: a Windows operating system.
 *
 * Enum values which specify the operating system running on the guest.
 */
typedef enum GTOSType {
	GT_OS_UNKNOWN,
	GT_OS_WINDOWS,
	GT_OS_LINUX,
	/* <private> */
	GT_OS_COUNT,
} GTOSType;

GTLoop  *gt_loop_new(const char *guest_name);
GTOSType gt_loop_get_ostype(GTLoop *loop);
gboolean gt_loop_set_cb(GTLoop *loop,
                        const char *kernel_func,
                        GTSyscallFunc syscall_cb,
                        GTSysretFunc sysret_cb,
                        void *user_data);
gboolean gt_loop_set_cbs(GTLoop *loop, const GTSyscallCallback callbacks[]);
void     gt_loop_run(GTLoop *loop);
void     gt_loop_quit(GTLoop *loop);
void     gt_loop_free(GTLoop *loop);

#endif
