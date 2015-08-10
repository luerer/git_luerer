#ifndef NET_MMT_MMTP_MLIST_H_
#define NET_MMT_MMTP_MLIST_H_

#include "mmt.h"
//http://blog.csdn.net/hopeyouknow/article/details/6716177

typedef struct mNode * mPNode;
typedef mPNode mPosition;
/*定义节点类型*/
typedef struct mNode
{
	mpu_buf_t *mpu_buf;		/*数据域*/
	mPNode previous; /*指向前驱*/
	mPNode next;		/*指向后继*/
}mNode;
/*定义链表类型*/
typedef struct
{
	mPNode head;		/*指向头节点*/
	mPNode tail;		/*指向尾节点*/
	int size;
}MPUList;

/*分配值为i的节点，并返回节点地址*/
mPosition mMakeNode(mpu_buf_t *mpu_buf);

/*释放p所指的节点*/
void mFreemNode(mPNode mp);

/*构造一个空的双向链表*/
MPUList* mInitList();

/*摧毁一个双向链表*/
void mDestroyList(MPUList *plist);

/*将一个链表置为空表，释放原链表节点空间*/
void mClearList(MPUList *plist);

/*返回头节点地址*/
mPosition mGetHead(MPUList *plist);

/*返回尾节点地址*/
mPosition mGetTail(MPUList *plist);

/*返回链表大小*/
int mGetSize(MPUList *plist);

/*返回p的直接后继位置*/
mPosition mGetNext(mPosition p);

/*返回p的直接前驱位置*/
mPosition mGetPrevious(mPosition p);

/*将pnode所指节点插入第一个节点之前*/
mPNode mInsFirst(MPUList *plist,mPNode mpnode);

/*将链表第一个节点删除并返回其地址*/
mPNode mDelFirst(MPUList *plist);

/*获得节点的数据项*/
unsigned char * mGetItem(mPosition p);

/*设置节点的数据项*/
void mSetItem(mPosition p,mpu_buf_t *mpu_buf);

/*删除链表中的尾节点并返回其地址，改变链表的尾指针指向新的尾节点*/
mPNode mRemove(MPUList *plist);

/*删除链表中的中间节点P的后继节点并返回地址*/
int mRemovePnext(MPUList *plist,mPosition p);

/*在链表中p位置之前插入新节点S*/
mPNode mInsBefore(MPUList *plist,mPosition p,mPNode ms);

/*在链表中p位置之后插入新节点s*/
mPNode mInsAfter(MPUList *plist,mPosition p,mPNode ms);

/*返回在链表中第i个节点的位置*/
mPNode mLocatePos(MPUList *plist,int i);

/*依次对链表中每个元素调用函数visit()*/
void mListTraverse(MPUList *plist,void(*visit)(mpu_buf_t*));

#endif

