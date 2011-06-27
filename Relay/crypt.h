#ifdef  __cplusplus
extern "C" {
#endif
#ifndef _CRYPT_H_
#define _CRYPT_H_
/********************************************************************

purpose:	
*********************************************************************/
#include <stdbool.h>


#if !defined BYTE
#define BYTE unsigned char
#endif
#if !defined UINT32
#define UINT32 unsigned int
#endif
#if !defined UINT
#define UINT unsigned int
#endif

#if !defined TRUE
#define TRUE true
#endif

#if !defined FALSE
#define FALSE false
#endif

#define TEAFLAG 'T'
#define XTEAFLAG 'X'
#define COMPRESSFLAG 'C'

	
    int XTEAEncrypt(char* pbyInBuffer, int nInBufferLength, char* pbyOutBuffer, int nOutBufferLength, char arrbyKey[16]);
    int XTEADecrypt(char* pbyInBuffer, int nInBufferLength, char* pbyOutBuffer, int nOutBufferLength, char arrbyKey[16]);
	

    static BYTE DEF_KEY[16] = {'A','3','4','F','9','7','7','1','E','C','1','0','B','C','A','5'};

    /// TEA����
    /*
    @param	pInBuffer IN			Ҫ���ܵ�buffer
    @param	nInBufferLen IN			Ҫ���ܵ�buffer�ĳ���
    @param	pOutBuffer IN			��ż��ܽ����buffer
    @param	pnOutBufferLen IN OUT	��ż��ܽ����buffer�ĳ���(IN)�����ؼ��ܽ����ʵ�ʴ�С(OUT)
    @param	pKey IN					���ܵ�key,16byte
    @param	uRounds IN				TEA��������������Խ��ȫ��Խ�ߣ��������ٶ�Խ��
    @remark	���ܽ����ȼ���ǰ�ĳ��ȴ�1-8��byte,���Խ��� len(��ż��ܽ����buffer) = len(Ҫ���ܵ�buffer) + 8
    */
    
    bool TEAEncrypt_(BYTE* pInBuffer, int nInBufferLen, BYTE* pOutBuffer, int* pnOutBufferLen, BYTE pKey[16], UINT uRounds);
    #define TEAEncrypt(pInBuffer, nInBufferLen, pOutBuffer, pnOutBufferLen,pKey)  \
	     TEAEncrypt_(pInBuffer, nInBufferLen, pOutBuffer, pnOutBufferLen,pKey,16)
    /// TEA����
    /*
    @param	pInBuffer		IN Ҫ���ܵ�buffer
    @param	nInBufferLen	IN Ҫ���ܵ�buffer�ĳ���
    @param	pOutBuffer		IN ��Ž��ܽ����buffer
    @param	pnOutBufferLen	IN OUT ��Ž��ܽ����buffer�ĳ���(IN)�����ؽ��ܽ����ʵ�ʴ�С(OUT)
    @param	pKey			IN ���ܵ�key,16byte,����ͼ���ʱ����ͬ
    @param	uRounds			IN ����,����ͼ���ʱ����ͬ
    @remark	��Ž��ܽ����buffer�ĳ��� ����Ҫ >= (Ҫ���ܵ�buffer�ĳ��� - 1)
    */
     
    bool TEADecrypt_(BYTE* pInBuffer, int nInBufferLen, BYTE* pOutBuffer, int* pnOutBufferLen, BYTE pKey[16], UINT uRounds );
     #define TEADecrypt(pInBuffer, nInBufferLen, pOutBuffer, pnOutBufferLen,pKey)  \
	     TEADecrypt_(pInBuffer, nInBufferLen, pOutBuffer, pnOutBufferLen,pKey,16)
	     
	int  TEAEncryptLen(int nInBufferLen);
      int TEADecryptLen(int nInBufferLen);

	bool StreamDecrypt_(const char *inbuf,int inbuflen,char *outbuf,int *outbuflen,char key[16],int type );
       #define StreamDecrypt(inbuf,inbuflen,outbuf,outbuflen,key) \
		StreamDecrypt_(inbuf,inbuflen,outbuf,outbuflen,key,1)
		
	bool StreamEncrypt_(const char *inbuf,int inbuflen,char *outbuf,int *outbuflen,char key[16],int type );
      #define StreamEncrypt(inbuf,inbuflen,outbuf,outbuflen,key) \
		StreamEncrypt_(inbuf,inbuflen,outbuf,outbuflen,key,1)
		
	bool StreamCompress(const char *inbuf,int inbuflen,char *outbuf,int *outbuflen);

	bool StreamUnCompress(const char *inbuf,int inbuflen,char *outbuf,int *outbuflen);

#endif
#ifdef  __cplusplus
}
#endif

