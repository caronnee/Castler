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


void Providers::Reset()
{
	_currentProvider = 0;
	for (int i = 0; i < _providers.size(); i++)
	{
		_providers[i]->SetPosition(-1);
	}
}

int Providers::Count()
{
	int ret = 0;
	for (int i = 0; i < _providers.size(); i++)
	{
		ret += _providers[i]->Count();
	}
	return ret;
}

bool Providers::SetPosition(const int & position)
{
	int tempPosition = position;
	_currentProvider = 0;
	while (_providers[_currentProvider]->Count() < tempPosition)
	{
		tempPosition -= _providers[_currentProvider]->Count();
		_currentProvider++;
		if (_currentProvider >= _providers.size())
			return false;
	}

	_providers[_currentProvider]->SetPosition(tempPosition);
	return true;
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


const QString Providers::Name()
{
	return _providers[_currentProvider]->Name();
}

void Providers::Get(int position, cv::Mat& frame)
{
	SetPosition(position);
	_providers[_currentProvider]->Frame(frame);
}

bool Providers::Frame(cv::Mat& frame)
{
	return _providers[_currentProvider]->Frame(frame);
}

bool Providers::Next()
{
	while (_currentProvider < _providers.size())
	{
		if (_providers[_currentProvider]->Next())
			return true;
		// no more images in current provider
		_currentProvider++;
	}
	return false;
}

void Providers::Create(const QString & str)
{
	QStringList list = str.split(";", QString::SkipEmptyParts);
	for (int i = 0; i < list.size(); i++)
	{
		IImageProvider * p = CreateProvider(list[i]);
		if (p)
			_providers.push_back(p);
	}
}