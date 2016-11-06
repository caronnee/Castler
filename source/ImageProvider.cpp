#include "imagesprovider.h"
#include "videoprovider.h"


IImageProvider * CreateProvider(const QString & source)
{
	IImageProvider * prov = VideoProvider::create(source);
	if (prov)
		return prov;
	return ImagesProvider::create(source);
}

