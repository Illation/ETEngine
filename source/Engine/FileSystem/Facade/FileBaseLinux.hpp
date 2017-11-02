FILE_HANDLE FILE_BASE::Open( const char * pathName, FILE_ACCESS_FLAGS accessFlags, FILE_ACCESS_MODE accessMode )
{
    return open( pathName, GetLinuxFileFlags(accessFlags, accessMode), 0644 );
}

bool FILE_BASE::Close( FILE_HANDLE handle )
{
    return close( handle );
}

bool FILE_BASE::ReadFile( FILE_HANDLE handle, std::string & content )
{
    char buffer[LINUX_FILE_BUFFER_SIZE];
    int32 ret_in = 0;
    while((ret_in = read (handle, &buffer, LINUX_FILE_BUFFER_SIZE)) > 0){}
    content = buffer;
    return true;;
}

bool FILE_BASE::WriteFile( FILE_HANDLE handle, const std::string & content )
{
    int32 result;
    result = write( handle, content.c_str(), content.length() );
    return result != -1;
}

int32 FILE_BASE::GetLinuxFileFlags( FILE_ACCESS_FLAGS flags, FILE_ACCESS_MODE mode )
{
    int32 result = 0;

    if ( flags.GetFlag( FILE_ACCESS_FLAGS::FLAGS::Create ) )
    {
        result |= O_CREAT;
    }
    if ( flags.GetFlag( FILE_ACCESS_FLAGS::FLAGS::Exists ) )
    {
        // nothing to do
    }
    if ( flags.GetFlag( FILE_ACCESS_FLAGS::FLAGS::Truncate ) )
    {
        result |= O_TRUNC;
    }

    switch ( mode )
    {
        case FILE_ACCESS_MODE::Read:
        result |= O_RDONLY;
        break;
        case FILE_ACCESS_MODE::Write:
        result |= O_WRONLY;
        break;
        case FILE_ACCESS_MODE::ReadWrite:
        result |= O_RDWR;
    }

    return result;
}
