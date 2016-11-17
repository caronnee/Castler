#include "imagesprovider.h"
#include "videoprovider.h"
#include <QStringList>

#define SUPPORTED_PROVIDERS(variable,str,XX)\
	XX(VideoProvider,str,variable)\
	XX(ImagesProvider,str,variable)\

#define TEST( provider, str, variable ) if ( provider::IsSupported(str)) return true;
#define TEST_CREATE( provider, str, variable ) if ( variable = provider::create(str)) return variable;


bool IsSupportedFile(const QString& str)
{
	IImageProvider * provider;
	SUPPORTED_PROVIDERS( provider, str, TEST);
	return false;
}

IImageProvider * CreateProvider(const QString & str)
{
	IImageProvider * provider;
	SUPPORTED_PROVIDERS(provider, str, TEST_CREATE);
	return NULL;
}


void Providers::Clear()
{
	for (int i = 0; i < _providers.size(); i++)
		delete _providers[i];
	_providers.clear();
	_currentProvider = 0;
}

bool Providers::IsValid() const
{
	return _providers.size() && _providers[0]->IsValid();
}


void Providers::Create(QString & str)
{
	QStringList list = str.split(";", QString::SkipEmptyParts);
	for (int i = 0; i < list.size(); i++)
	{
		IImageProvider * p = CreateProvider(list[i]);
		if (p)
			_providers.push_back(p);
	}
}