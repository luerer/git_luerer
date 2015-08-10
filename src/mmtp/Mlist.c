#include "Mlist.h"
#include<malloc.h>
#include<stdlib.h>
/*分配值为i的节点，并返回节点地址*/
mPosition mMakeNode(mpu_buf_t *mpu_buf)
{
	mPNode p = NULL;
	p = (mPNode)malloc(sizeof(mNode));
	p->mpu_buf=(mpu_buf_t*)malloc(1500*sizeof(unsigned char));
	memset((void*)p->mpu_buf,0,1500);
	if(p!=NULL)
	{
		//p->mpu_buf = i;
		memcpy((void*)p->mpu_buf , &mpu_buf[0] , 1500);
		p->previous = NULL;
		p->next = NULL;
	}
	return p;
}
/*释放p所指的节点*/
void mFreemNode(mPNode p)
{
	 free(p);
}
/*构造一个空的双向链表*/
MPUList * mInitList()
{
    mpu_buf_t tmpu;
	MPUList *plist = (MPUList *)malloc(sizeof(MPUList));
    memset(&tmpu, 0, sizeof(tmpu));
	mPNode head = mMakeNode(&tmpu);
	if(plist!=NULL)
	{
		if(head!=NULL)
		{
			plist->head = head;
			plist->tail = head;
			plist->size = 0;
		}
		else
			return NULL;
	}
	return plist;
}

/*摧毁一个双向链表*/
void mDestroyList(MPUList *plist)
{
	mClearList(plist);
	free(mGetHead(plist));
	free(plist);
}

/*判断链表是否为空表*/
int mIsEmpty(MPUList *plist)
{
	if(mGetSize(plist)==0&&mGetTail(plist)==mGetHead(plist))
		return 1;
	else
		return 0;
}
/*将一个链表置为空表，释放原链表节点空间*/
void mClearList(MPUList *plist)
{
	mPNode temp,p;
	p = mGetTail(plist);
	while(!mIsEmpty(plist))
	{
		temp = mGetPrevious(p);
		free(p->mpu_buf);
		mFreemNode(p);
		p = temp;
		plist->tail = temp;
		plist->size--;
		printf("size %d\n",plist->size);
	}
}

/*返回头节点地址*/
mPosition mGetHead(MPUList *plist)
{
	return plist->head;
}

/*返回尾节点地址*/
mPosition mGetTail(MPUList *plist)
{
	return plist->tail;
}

/*返回链表大小*/
int mGetSize(MPUList *plist)
{
	return plist->size;
}

/*返回p的直接后继位置*/
mPosition mGetNext(mPosition p)
{
	return p->next;
}

/*返回p的直接前驱位置*/
mPosition mGetPrevious(mPosition p)
{
	return p->previous;
}

/*将pnode所指节点插入第一个节点之前*/
mPNode mInsFirst(MPUList *plist,mPNode pnode)
{
	mPosition head = mGetHead(plist);

	if(mIsEmpty(plist))
		plist->tail = pnode;
	plist->size++;

	pnode->next = head->next;
	pnode->previous = head;

	if(head->next!=NULL)
		head->next->previous = pnode;
	head->next = pnode;

	return pnode;
}

/*将链表第一个节点删除,返回该节点的地址,删除数据*/
mPNode mDelFirst(MPUList *plist)
{
	mPosition head = mGetHead(plist);

	mPosition p=head->next;

	if(p!=NULL)
	{
		if(p==mGetTail(plist))
		{
			plist->tail = p->previous;
			head->next = NULL;
		}
		else
		{
			head->next = p->next;
			head->next->previous = head;
		}
		plist->size--;
		free(p->mpu_buf);

	}
	return p;
}
/*将链表第一个节点删除,返回该节点的地址,不删除数据*/
mPNode mDelFirst2(MPUList *plist)
{
	mPosition head = mGetHead(plist);
	mPosition p=head->next;
	if(p!=NULL)
	{
		if(p==mGetTail(plist))
			plist->tail = p->previous;
		head->next = p->next;
		head->next->previous = head;
		plist->size--;

	}
	return p;
}

/*获得节点的数据项*/
unsigned char* mGetItem(mPosition p)
{
	return (unsigned char*)p->mpu_buf;
}

/*设置节点的数据项*/
void mSetItem(mPosition p,mpu_buf_t *mpu_buf)
{
	p->mpu_buf = mpu_buf;
}

/*删除链表中的尾节点并返回地址，改变链表的尾指针指向新的尾节点*/
mPNode mRemove(MPUList *plist)
{
	mPosition p=NULL;
	if(mIsEmpty(plist))
		return NULL;
	else
	{
		p = mGetTail(plist);
		free(p->mpu_buf);

		plist->tail = p->previous;

		p->previous->next = NULL;
		printf("hehe\n");
		plist->size--;

		return p;
	}
}
/*删除链表中的中间节点P的后继节点并返回地址*/
int mRemovePnext(MPUList *plist,mPosition p)
{
	mPosition q=p->next;
	if(mIsEmpty(plist))
		return -1;
	else
	{
		p->next=q->next;  // 图2-10中步骤①①
		q->next->previous=p;  //图 2-10 中步骤②
		free (q->mpu_buf);
		free (q) ; //释放结点空间
		plist->size--;
		return 0;
	}
}

/*在链表中p位置之前插入新节点s*/
mPNode mInsBefore(MPUList *plist,mPosition p,mPNode s)
{
	s->previous = p->previous;
	s->next = p;
	p->previous->next = s;
	p->previous = s;

	plist->size++;
	return s;
}
/*在链表中p位置之后插入新节点s*/
mPNode mInsAfter(MPUList *plist,mPosition p,mPNode s)
{
	s->next = p->next;
	s->previous = p;

	if(p->next != NULL)
		p->next->previous = s;
	p->next = s;

	if(p == mGetTail(plist))
		plist->tail = s;

	plist->size++;
	return s;
}

/*返回在链表中第i个节点的位置*/
mPNode mLocatePos(MPUList *plist,int i)
{
	int cnt = 0;
	mPosition p = mGetHead(plist);
	if(i>mGetSize(plist)||i<1)
		return NULL;

	while(++cnt<=i)
	{
		p=p->next;
	}

	return p;
}

/*依次对链表中每个元素调用函数visit()*/
void mListTraverse(MPUList *plist,void(*visit)(mpu_buf_t*))
{
	mPosition p = mGetHead(plist);
	if(mIsEmpty(plist))
		exit(0);
	else
	{

		while(p->next!=NULL)
		{
			p = p->next;
			visit(p->mpu_buf);
		}
	}
}


