#ifndef __IPCTOOLS_LOGGER_H__
#define __IPCTOOLS_LOGGER_H__

#include "allocator_shm.h"
#include "shared_queue.h"

#define LOGGER_MAX_MESSAGE_SIZE (256)

typedef ipt_shared_queue_node_t ipt_logger_node_t;
typedef struct ipt_logger_t ipt_logger_t;
typedef struct ipt_logger_message_t ipt_logger_message_t;
typedef enum ipt_log_category_mask_t ipt_log_category_mask_t;
typedef enum ipt_log_level_mask_t ipt_log_level_mask_t;

enum ipt_log_category_mask_t
{
	/* Modules */
	IPT_MODULE_FAULT   = 1<<0,
	IPT_MODULE_MODULE  = 1<<1,
	IPT_MODULE_ALL     = 0xffffffff,

	/* Processes */
	IPT_PROCESS_PROCESS_ONE    = 1<<4
};

enum ipt_log_level_mask_t
{
	IPT_LEVEL_EMERG    = 1<<0,
	IPT_LEVEL_ALERT    = 1<<1,
	IPT_LEVEL_CRIT     = 1<<2,
	IPT_LEVEL_ERROR    = 1<<3,
	IPT_LEVEL_WARNING  = 1<<4,
	IPT_LEVEL_NOTICE   = 1<<5,
	IPT_LEVEL_INFO     = 1<<6,
	IPT_LEVEL_DEBUG    = 1<<7,
	IPT_LEVEL_ALL      = 0xffffffff 
};

/**
 * @struct ipt_logger_message_t
 *
 * @brief This wrapper for messages used by logging system.
 *
 * Each message sent to the logging subystem will be wrapped
 * with standard informaton.
 *
 */
struct ipt_logger_message_t
{
   /**
    *  Node linkage for instrusive shared list
    */
   ipt_logger_node_t node;

   /**
    * Category mask. 
    *
    * This does not map to syslog in RFC3164
    * and are defined internally.
    *
    * If a message is sent to a remote syslog server
    * it will be mapped to Facility 2 (user messages)
    * defined in 
    */
   ipt_log_category_mask_t cmask;

   /**
    * Level mask.
    *
    * This maps directly to syslog severity level defined
    * int RFC 3164.
    */
   ipt_log_level_mask_t lmask;

   /**
    * Name of process that originated message
    */
   char originator[32];

   /**
    * Time at which the message was generated.
    *
    * where the format is * [RFC 3164]:[milliseconds]
    *
    * milliseconds is added for local logging, but stripped when
    * sending to remote syslog server.
    *
    */
   char time[32];

   /**
    * Message generated by the process.
    */
   char message[LOGGER_MAX_MESSAGE_SIZE];
};

/**
 * @struct ipt_logger_t
 *
 * @brief The logger object.
 *
 */

struct ipt_logger_t
{
       /**
         * Enqueue a log message.
         *
         * @param[in] this The this pointer.
         * @param[in] category_mask The category of the message. 
         * @param[in] level_mask The level of the message. 
         * @param[in] originator The originator of the message. 
         * @param[in] fmt The variable argument format of the message. 
         *
         * @retval 0 Succeeded. 
         * @retval 1 Failed.
         */
	int (*enqueue)(ipt_logger_t *this, ipt_log_category_mask_t category_mask, ipt_log_level_mask_t level_mask, const char *originator, const char *fmt, ...);

       /**
         * Dequeue a log message. This will wait until a message is available.
         *
         * @param[in] this The this pointer.
         *
         * @retval !NULL The pointer to the log message.
         * @retval NULL Failed.
         */
	ipt_logger_message_t * (*dequeue)(ipt_logger_t *this);

       /**
         * Dequeue a log message. This will wait the specified time. 
         *
         * @param[in] this The this pointer.
         * @param[in] tv The time to wait for a message to be placed on the queue.
         *
         * @retval !NULL The pointer to the log message.
         * @retval NULL Failed.
         */
	ipt_logger_message_t * (*dequeue_timed)(ipt_logger_t *this, ipt_time_value_t *tv);

       /**
         * Return the allocator used by the logger.
         *
         * @param[in] this The this pointer.
         *
         * @retval !NULL The pointer to the allocator.
         * @retval NULL Failed.
         */ 
   	ipt_allocator_t* (*get_allocator)(ipt_logger_t *this);

       /**
         * Check to see if a category is set in the category mask.
         *
         * @param[in] this The this pointer.
         * @param[in] category The category mask to be checked against.
         *
         * @retval 0 Is not set..
         * @retval 1 Is set. 
         */
	int (*is_category_set)(ipt_logger_t *this, enum ipt_log_category_mask_t category);

       /**
         * Check to see if a level is set in the category mask.
         *
         * @param[in] this The this pointer.
         * @param[in] level The level mask to be checked against.
         *
         * @retval 0 Is not set..
         * @retval 1 Is set.   
         */
	int (*is_level_set)(ipt_logger_t *this, enum ipt_log_level_mask_t level);

       /**
         * Get the category mask.
         *
         * @param[in] this The this pointer.
         *
         * @retval mask The mask returned. 
         */
	ipt_log_category_mask_t (*get_category_mask)(ipt_logger_t *this);

       /**
         * Get the level mask.
         *
         * @param[in] this The this pointer.
         *
         * @retval mask The mask returned.
         */
	ipt_log_level_mask_t (*get_level_mask)(ipt_logger_t *this);

       /**
         * Add a category to the mask.
         *
         * @param[in] this The this pointer.
         * @param[in] category The category mask.
         *
         */
	void (*set_category)(ipt_logger_t *this, enum ipt_log_category_mask_t category);

       /**
         * Add a level to the mask.
         *
         * @param[in] this The this pointer.
         * @param[in] level The level mask.
         *
         */
	void (*set_level)(ipt_logger_t *this, enum ipt_log_level_mask_t level);

       /**
         * Unset a category.
         *
         * @param[in] this The this pointer.
         * @param[in] category The category mask.
         *
         */
	void (*unset_category)(ipt_logger_t *this, enum ipt_log_category_mask_t category);

       /**
         * Unset a level.
         *
         * @param[in] this The this pointer.
         * @param[in] level The level mask.
         *
         */
	void (*unset_level)(ipt_logger_t *this, enum ipt_log_level_mask_t level);

       /**
         * Get the file descriptor of the named pipe used to listen for enqueue events.
         *
         * @param[in] this The this pointer.
         *
         * @retval int The file descriptor of the named pipe
         */
	int (*get_fd)(ipt_logger_t *this);

       /**
         *  Free a message.
         *
         * @param[in] this The this pointer.
         * @param[in] msg The msg pointer.
         *
         */ 
	void (*free)(ipt_logger_t *this, void *msg);

       /**
         *  Get the syslog priority.
         *
         * @param[in] this The this pointer.
         * @param[in] msg The msg pointer.
         *
         */ 
	int (*syslog_priority)(ipt_logger_t *this, ipt_logger_message_t *msg);

};

/**
  * Logger constructor 
  *
  * @param[in] name The name of the logger. The logger will be registered with the allocator under this name.
  * @param[in] alloc_ptr The allocator to be used by the logger.
  *
  * @retval NULL Failed.
  * @retval !NULL Succeeded. 
  */
ipt_logger_t *ipt_logger_create(const char *name, ipt_allocator_t *alloc_ptr);

/**
  * Logger constructor
  *
  * @param[in] name The name of the logger. The logger will be registered with the allocator under this name.
  * @param[in] alloc_ptr The allocator to be used by the logger.
  *
  * @retval NULL Failed.
  * @retval !NULL Succeeded.
  */
ipt_logger_t * ipt_logger_attach(const char *name, ipt_allocator_t *alloc_ptr);

/**
  * Walk the log messages. 
  *
  * @param[in] this The this pointer..
  * @param[in] func The callback that will be called with each message.
  * @param[in] in_ptr Pointer to object that will be passed through to the callback..
  *
  * @retval NULL Failed.
  * @retval !NULL Succeeded.
  */
void ipt_logger_for_each(ipt_logger_t *this, void (*func)(const ipt_logger_message_t * const, void *), void *in_ptr);

/**
  * Dump the log messages.
  *
  * @param[in] msg The msg to be dumped
  */
void ipt_logger_dump_message(ipt_logger_message_t *msg);

/**
  * Convenience function.
  *
  * @param[in] this The this pointer. 
  * @param[in] category The category of the message. 
  * @param[in] level The level of the message. 
  * @param[in] msg The message. 
  */
void ipt_log_message(ipt_logger_t* logger, int category, int level, char* msg, ...);

#endif
