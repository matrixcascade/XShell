#include "gdiplus.h" 
using namespace Gdiplus;
#pragma comment(lib,"gdiplus.lib")

bool CaptureScreen( UINT quality)
{
	HDC hDCSrc = ::GetDC(NULL);
	int nBitPerPixel = GetDeviceCaps(hDCSrc, BITSPIXEL);//像素位
	int nWidth = GetDeviceCaps(hDCSrc, HORZRES);//// 得到当前显示设备的水平像素数
	int nHeight = GetDeviceCaps(hDCSrc, VERTRES);
	CImage cImage;//使用CImage能省好多截图的代码
	cImage.Create(nWidth, nHeight, nBitPerPixel);
	BitBlt(cImage.GetDC(), 0, 0, nWidth, nHeight, hDCSrc, 0, 0, SRCCOPY);
	::ReleaseDC(NULL, hDCSrc);
	cImage.ReleaseDC();//截图的代码到这里就结束了

	COleStreamFile cImgStream;
	cImgStream.CreateMemoryStream(NULL);
	cImage.Save(cImgStream.GetStream(), Gdiplus::ImageFormatBMP);//将2进制数据写入流
	Image image(cImgStream.GetStream());//从流创建Graphics::Image对象
	cImgStream.Close();

	CLSID encoderClsid;
	GetEncoderClsid(L"image/jpeg", &encoderClsid);//获取编码CLSID
	EncoderParameters encoderParameters;//下面这几样编码的参数我只知道一个.Value为压缩率，其他的不懂。
	encoderParameters.Count = 1; 
	encoderParameters.Parameter[0].Guid = EncoderQuality; 
	encoderParameters.Parameter[0].Type = EncoderParameterValueTypeLong; 
	encoderParameters.Parameter[0].NumberOfValues = 1; 
	encoderParameters.Parameter[0].Value = &quality;
	image.Save(L"error.jpg", &encoderClsid, &encoderParameters); //也可直接写入图片文件
	 
	return true;
}