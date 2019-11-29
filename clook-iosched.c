/*
 * Author: Leonardo Canizares
 * elevator clook
 */
#include <linux/blkdev.h>
#include <linux/elevator.h>
#include <linux/bio.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/init.h>

int dh = -1; //keeps track of diskhead position

struct clook_data {
	struct list_head queue;
};

static void clook_merged_requests(struct request_queue *q, struct request *rq,
				 struct request *next)
{
	list_del_init(&next->queuelist);
}

/*
 * clook_dispatch modified to implement clook algorithm
 * 
 */
static int clook_dispatch(struct request_queue *q, int force)
{
	struct clook_data *nd = q->elevator->elevator_data;
	char direction;

	if (!list_empty(&nd->queue)) {
		struct request *rq;
		rq = list_entry(nd->queue.next, struct request, queuelist);
		list_del_init(&rq->queuelist);
		elv_dispatch_sort(q, rq);
		dh = blk_rq_pos(rq);//update diskhead position

		//print statement formating
		if(rq_data_dir(rq)){
			direction = 'W';
		} else {
			direction = 'R';
		}
	
		printk("[CLOOK] dsp %c %lu\n", direction, blk_rq_pos(rq));

		return 1;
	}
	return 0;
}

/*
 * clook_add_request modified to implement clook algorithm
 * 
 */
static void clook_add_request(struct request_queue *q, struct request *rq)
{
	struct clook_data *nd = q->elevator->elevator_data;
	struct list_head *cur = NULL; //pointer for current entry
	char direction;

	//iterate through request queue
	list_for_each(cur, &nd->queue){

		struct request *entry = list_entry(cur, struct request, queuelist);

		if (blk_rq_pos(rq) > dh)
		{
			// If the current position less than diskhead OR rq position less than current position
			//break and add to current position
			if(blk_rq_pos(entry) < dh || blk_rq_pos(rq) < blk_rq_pos(entry))
				break;
			
		} else {
			// current position less than disk head
			// If the current position less than diskhead AND rq position less than current position 
			//break and add to cur position
			if(blk_rq_pos(entry) < dh && blk_rq_pos(rq) < blk_rq_pos(entry))
				break;
		}

	}

	//add request to queue
	list_add_tail(&rq->queuelist, cur);

	//print statement formating
	if(rq_data_dir(rq)){
		direction = 'W';
	} else {
		direction = 'R';
	}

	//added debug print statement
	printk("[CLOOK] add %c %lu\n", direction, blk_rq_pos(rq));

	
}

static struct request *
clook_former_request(struct request_queue *q, struct request *rq)
{
	struct clook_data *nd = q->elevator->elevator_data;

	if (rq->queuelist.prev == &nd->queue)
		return NULL;
	return list_entry(rq->queuelist.prev, struct request, queuelist);
}

static struct request *
clook_latter_request(struct request_queue *q, struct request *rq)
{
	struct clook_data *nd = q->elevator->elevator_data;

	if (rq->queuelist.next == &nd->queue)
		return NULL;
	return list_entry(rq->queuelist.next, struct request, queuelist);
}

static int clook_init_queue(struct request_queue *q)
{
	struct clook_data *nd;

	nd = kmalloc_node(sizeof(*nd), GFP_KERNEL, q->node);
	if (!nd)
		return -ENOMEM;

	INIT_LIST_HEAD(&nd->queue);
	q->elevator->elevator_data = nd;
	return 0;
}

static void clook_exit_queue(struct elevator_queue *e)
{
	struct clook_data *nd = e->elevator_data;

	BUG_ON(!list_empty(&nd->queue));
	kfree(nd);
}

static struct elevator_type elevator_clook = {
	.ops = {
		.elevator_merge_req_fn		= clook_merged_requests,
		.elevator_dispatch_fn		= clook_dispatch,
		.elevator_add_req_fn		= clook_add_request,
		.elevator_former_req_fn		= clook_former_request,
		.elevator_latter_req_fn		= clook_latter_request,
		.elevator_init_fn		= clook_init_queue,
		.elevator_exit_fn		= clook_exit_queue,
	},
	.elevator_name = "clook",
	.elevator_owner = THIS_MODULE,
};

static int __init clook_init(void)
{
	return elv_register(&elevator_clook);
}

static void __exit clook_exit(void)
{
	elv_unregister(&elevator_clook);
}

module_init(clook_init);
module_exit(clook_exit);


MODULE_AUTHOR("Leonardo Canizares");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("C-Look IO scheduler");
