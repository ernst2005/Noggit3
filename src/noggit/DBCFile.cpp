#include <noggit/DBCFile.h>

#include <string>

#include <noggit/mpq/file.h>
#include <noggit/Log.h>

struct dbc_header
{
  uint32_t magic;
  uint32_t recordCount;
  uint32_t fieldCount;
  uint32_t recordSize;
  uint32_t stringSize;
};

DBCFile::DBCFile (const QString& filename)
  : _filename (filename)
  , data (NULL)
{
}

void DBCFile::open()
{
  noggit::mpq::file f (_filename);

  LogDebug << "Opening DBC " << qPrintable (_filename) << std::endl;

  dbc_header header;
  f.read (&header, sizeof (header));

  //! \note Yup, in these files, they store the magic as string, not uint32_t.
  assert (header.magic == 'CBDW');
  assert (fieldCount * 4 == recordSize);

  recordCount = header.recordCount;
  fieldCount = header.fieldCount;
  recordSize = header.recordSize;
  stringSize = header.stringSize;

  data = new unsigned char[recordSize * recordCount + stringSize];
  stringTable = data + recordSize * recordCount;
  f.read (data, recordSize * recordCount + stringSize);
  f.close();
}

DBCFile::~DBCFile()
{
  if( data )
  {
    delete[] data;
    data = NULL;
  }
}


