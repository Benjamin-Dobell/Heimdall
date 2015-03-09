#import "GZipFile.h"

using namespace HeimdallFrontend;

GZipFile::GZipFile(const QString& path) :
	file(path),
	gzFile(nullptr)
{
}

GZipFile::~GZipFile()
{
	Close();

	if (temporary)
	{
		file.remove();
	}
}

bool GZipFile::Open(Mode mode)
{
	if (!file.isOpen() && !file.open(mode == GZipFile::ReadOnly ? QFile::ReadOnly : QFile::WriteOnly))
	{
		return (false);
	}

	gzFile = gzdopen(file.handle(), mode == GZipFile::ReadOnly ? "rb" : "wb");
	return (gzFile != nullptr);
}

void GZipFile::Close()
{
	file.close();
	gzclose(gzFile);
}

int GZipFile::Read(void *buffer, int length)
{
	return (length >= 0 && !file.isWritable() ? gzread(gzFile, buffer, length) : -1);
}

bool GZipFile::Write(void *buffer, int length)
{
	return (length >= 0 && gzwrite(gzFile, buffer, length) == length);
}

qint64 GZipFile::Offset() const
{
	return gzoffset(gzFile);
}
