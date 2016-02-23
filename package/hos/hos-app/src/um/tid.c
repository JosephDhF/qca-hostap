/******************************************************************************
  �� �� ��   : tid.c
  ��    ��   : wenjue
  ��������   : 2014��11��19��
  ��������   : umģ����ա��洢��ά����ɾ���豸��Ϣ
******************************************************************************/
#include <pthread.h>

#include "utils.h"
#include "tid.h"
#include "um.h"

#define ITD_PORT 5600
#define TID_RECV_BUF_MAXLEN 256

static struct devinfohead *g_pstdevinfohead = NULL;
/*****************************************************************************
 �� �� ��  : tid_rwlock_rdlock
 ��������  : �������豸��Ϣ����
 �������  : ��
 �������  : ��
 �� �� ֵ  : int == 0 ���سɹ�
                 != 0 ����ʧ��
 ��   ��   : wenjue
*****************************************************************************/
int tid_rwlock_rdlock(void)
{
    return pthread_rwlock_rdlock(&(g_pstdevinfohead->rw_lock));
}

/*****************************************************************************
 �� �� ��  : tid_rwlock_wrlock
 ��������  : д�����豸��Ϣ����
 �������  : ��
 �������  : ��
 �� �� ֵ  : int == 0 ���سɹ�
                 != 0 ����ʧ��
 ��   ��   : wenjue
*****************************************************************************/
int tid_rwlock_wrlock(void)
{
    return pthread_rwlock_wrlock(&(g_pstdevinfohead->rw_lock));
}

/*****************************************************************************
 �� �� ��  : tid_rwlock_unlock
 ��������  : �����豸��Ϣ����
 �������  : ��
 �������  : ��
 �� �� ֵ  : int == 0 ���سɹ�
                 != 0 ����ʧ��
 ��   ��   : wenjue
*****************************************************************************/
int tid_rwlock_unlock(void)
{
    return pthread_rwlock_unlock(&(g_pstdevinfohead->rw_lock));
}

/*****************************************************************************
 �� �� ��  : tid_init_devinfohead
 ��������  : ��ʼ������ͷ
 �������  : ��
 �������  : ��
 �� �� ֵ  : int == 0 ���سɹ�
                 != 0 ����ʧ��
 ��   ��   : wenjue
*****************************************************************************/
static int tid_init_devinfohead(void)
{
    int ret = -1;
    
    if (NULL == g_pstdevinfohead)
    {
        g_pstdevinfohead = malloc(sizeof(struct devinfohead));
        if (NULL != g_pstdevinfohead)
        {
            g_pstdevinfohead->next = NULL;
            ret = pthread_rwlock_init(&(g_pstdevinfohead->rw_lock), NULL);
        }
    }
    
    return ret;
}

/*****************************************************************************
 �� �� ��  : tid_fini_devinfohead
 ��������  : ȥ��ʼ������ͷ
 �������  : ��
 �������  : ��
 �� �� ֵ  : ��
 ��   ��   : wenjue
*****************************************************************************/
static void tid_fini_devinfohead(void)
{
    if (NULL != g_pstdevinfohead)
    {
        pthread_rwlock_destroy(&(g_pstdevinfohead->rw_lock));
        free(g_pstdevinfohead);
        g_pstdevinfohead = NULL;
    }

    return;
}

/*****************************************************************************
 �� �� ��  : tid_add_devinfonode
 ��������  : ���һ���豸��Ϣ�ڵ�
 �������  : const struct devinfo *pstdevinfo �豸��Ϣ�ṹ
 �������  : ��
 �� �� ֵ  : ��
 ��   ��   : wenjue
*****************************************************************************/
static void tid_add_devinfonode(const struct devinfo *pstdevinfo)
{
    struct devinfonode *pstdevinfonode = NULL;

    if (NULL == pstdevinfo || NULL == g_pstdevinfohead)
    {
        return;
    }

    pstdevinfonode = malloc(sizeof(struct devinfonode));
    if (NULL == pstdevinfonode)
    {
        return;
    }
    memcpy(&pstdevinfonode->stdevinfo, pstdevinfo, sizeof(pstdevinfonode->stdevinfo));
    pstdevinfonode->aging = UM_AGING_TIMES;
    if (g_pstdevinfohead->next != NULL)
    {
        g_pstdevinfohead->next->prev = pstdevinfonode;
    }
    pstdevinfonode->next = g_pstdevinfohead->next;
    pstdevinfonode->prev = NULL;
    g_pstdevinfohead->next = pstdevinfonode;

    return;
}

/*****************************************************************************
 �� �� ��  : tid_update_devinfonode
 ��������  : �����豸��Ϣ�����ж��ϻ��ֶ�aging������ʱ��û����ĳ���豸��Ϣ��
             ��ɾ�����豸��Ϣ
 �������  : ��
 �������  : ��
 �� �� ֵ  : ��
 ��   ��   : wenjue
*****************************************************************************/
void tid_update_devinfonode(void)
{
    struct devinfonode *pstcurnode = NULL;

    if (NULL == g_pstdevinfohead)
    {
        return;
    }

    pstcurnode = g_pstdevinfohead->next;
    while(pstcurnode != NULL)
    {
        if (pstcurnode->aging <= 0)
        {
            if (pstcurnode->prev == NULL && pstcurnode->next == NULL)
            {
                g_pstdevinfohead->next = NULL;
            }
            else if (pstcurnode->prev == NULL)
            {
                g_pstdevinfohead->next = pstcurnode->next;
                pstcurnode->next->prev = NULL;
            }
            else if (pstcurnode->next == NULL)
            {
                pstcurnode->prev->next = NULL;
            }
            else
            {
                pstcurnode->prev->next = pstcurnode->next;
                pstcurnode->next->prev = pstcurnode->prev;
            }
            free(pstcurnode);
            pstcurnode = NULL;
            break;
        }
        pstcurnode->aging--;
        pstcurnode = pstcurnode->next;
    }

    return;
}

/*****************************************************************************
 �� �� ��  : tid_get_devinfo
 ��������  : ��ȡ�豸��Ϣ
 �������  : const char *mac����ʶ�豸��Ϣ��MAC��ַ
 �������  : struct devinfo *pstdevinfo�� ��ȡ�õ����豸��Ϣ����
             ע�⣬�˴���������������ṩ�ռ�
 �� �� ֵ  : int == 0 ��ȡ�ɹ�
             int != 0 ��ȡʧ��
 ��   ��   : wenjue
*****************************************************************************/
int tid_get_devinfo(struct devinfo *pstdevinfo, const char *mac)
{
    struct devinfonode *pstcurnode = NULL;
    int ret = -1;

    if (NULL == g_pstdevinfohead || NULL == mac || NULL == pstdevinfo)
    {
        return ret;
    }

    pstcurnode = g_pstdevinfohead->next;
    while(pstcurnode != NULL)
    {
        if (0 == strcasecmp(pstcurnode->stdevinfo.mac, mac))
        {
            memcpy(pstdevinfo, &(pstcurnode->stdevinfo), sizeof(struct devinfo));
            pstcurnode->aging = UM_AGING_TIMES;
            ret = 0;
            break;
        }
        pstcurnode = pstcurnode->next;
    }

    return ret;
}

/*****************************************************************************
 �� �� ��  : tid_modify_devinfo
 ��������  : �޸�ĳ���豸��Ϣ
 �������  : const struct devinfo *pstdevinfo�������豸��Ϣ�޸ĵ�����
 �������  : ��
 �� �� ֵ  : int == 0 ���ӱ���ڴ��豸��Ϣ���޸ĳɹ�
             int != 0 �����в����ڴ��豸��Ϣ���޸�ʧ��
             ע�⣬���޸������������е��豸��Ϣ��ͬ�����سɹ�
 ��   ��   : wenjue
*****************************************************************************/
static int tid_modify_devinfo(const struct devinfo *pstdevinfo)
{
    struct devinfonode *pstcurnode = NULL;
    int ret = -1;

    if (NULL == g_pstdevinfohead || NULL == pstdevinfo)
    {
        return ret;
    }

    pstcurnode = g_pstdevinfohead->next;
    while(pstcurnode != NULL)
    {
        if (0 == strcasecmp(pstcurnode->stdevinfo.mac, pstdevinfo->mac))
        {
            if (pstdevinfo->hostname[0] != 0)
            {
                memcpy(pstcurnode->stdevinfo.hostname, pstdevinfo->hostname, sizeof(pstcurnode->stdevinfo.hostname));
            }
            if (pstdevinfo->devtype[0] != 0)
            {
               memcpy(pstcurnode->stdevinfo.devtype, pstdevinfo->devtype, sizeof(pstcurnode->stdevinfo.devtype));
            }
            if (pstdevinfo->devmodel[0] != 0)
            {
                memcpy(pstcurnode->stdevinfo.devmodel, pstdevinfo->devmodel, sizeof(pstcurnode->stdevinfo.devmodel));
            }
            if (pstdevinfo->cputype[0] != 0)
            {
                memcpy(pstcurnode->stdevinfo.cputype, pstdevinfo->cputype, sizeof(pstcurnode->stdevinfo.cputype));
            }
            if (pstdevinfo->ostype[0] != 0)
            {
                memcpy(pstcurnode->stdevinfo.ostype, pstdevinfo->ostype, sizeof(pstcurnode->stdevinfo.ostype));
            }
            if (pstdevinfo->ipaddr[0] != 0)
            {
                memcpy(pstcurnode->stdevinfo.ipaddr, pstdevinfo->ipaddr, sizeof(pstcurnode->stdevinfo.ipaddr));
            }
            ret = 0;
            pstcurnode->aging = UM_AGING_TIMES;
            break;
        }
        pstcurnode = pstcurnode->next;
    }
    
    return ret;
}

/*****************************************************************************
 �� �� ��  : tid_destroy_devinfo
 ��������  : ���������豸��Ϣ����
 �������  : ��
 �������  : ��
 �� �� ֵ  : ��
 ��   ��   : wenjue
*****************************************************************************/
void tid_destroy_devinfo(void)
{
    struct devinfonode *pstcurnode = NULL;
    struct devinfonode *freenode = NULL;

    if (NULL == g_pstdevinfohead)
    {
        return;
    }

    pstcurnode = g_pstdevinfohead->next;
    while(pstcurnode != NULL)
    {
        freenode = pstcurnode;
        pstcurnode = pstcurnode->next;
        free(freenode);
    }
    if (NULL != pstcurnode)
    {
        free(pstcurnode);
    }
    
    return;
}


/*****************************************************************************
 �� �� ��  : tid_pthreadhandle
 ��������  : umģ����̴߳������������ռ����洢�豸��Ϣ
 �������  : void *data���̴߳����������ݲ���
 �������  : ��
 �� �� ֵ  : void *�̴߳���������ֵ
 ��   ��   : wenjue
*****************************************************************************/
void *tid_pthreadhandle(void *data)
{
    struct sockaddr_in serv_addr;
    struct devinfo *pdevinfo = NULL;
    char buf[TID_RECV_BUF_MAXLEN] = {0};
    int socketfd = -1;
    int recvlen = 0;
    int ret = -1;
    
    if((socketfd = socket(AF_INET,SOCK_DGRAM,0)) < 0)
    {
        debug_tid_error("[um]: socket create failed, um exit!\r\n");     
        exit(1);
    } 
    if(0 != tid_init_devinfohead())
    {
        close(socketfd); 
        debug_tid_error("[um]: Devinfohead init failed, um exit!\r\n");    
        exit(1);
    }
    memset(&serv_addr,0,sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(ITD_PORT);
    if(bind(socketfd,(struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        debug_tid_error("[um]: socket bind failed, um exit!\r\n");     
        close(socketfd);     
        exit(1);
    }

    while (1)
    {
        recvlen = recvfrom(socketfd, buf, sizeof(buf), 0, NULL, NULL);
        if (recvlen == sizeof(struct devinfo))
        {
            debug_tid_trace("[um]: RecvMsg From Tid Success!\r\n");
            pdevinfo = (struct devinfo *)buf;
            tid_rwlock_wrlock();
            ret = tid_modify_devinfo(pdevinfo);
            if (ret != 0)
            {
                tid_add_devinfonode(pdevinfo);
            }
            tid_rwlock_unlock();
        }
        else
        {
            debug_tid_waring("[um]: RecvMsg From Tid Failed!\r\n");
        }
    }
    tid_fini_devinfohead();
    close(socketfd);

    return NULL;
}

