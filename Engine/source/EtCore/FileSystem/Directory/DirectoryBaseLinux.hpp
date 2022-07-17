#include <sys/types.h>
#include <dirent.h>
#include <iostream>


namespace et {
namespace core {


bool Directory::Mount(bool recursive)
{
    if(!m_IsMounted)
    {
        DIR *dp;
        dirent *dirp;

        if((dp = opendir((GetPath()+m_Filename).c_str())) == NULL)
        {
			ET_TRACE_E(ET_CTX_CORE, "linux dir '%s' failed opening", (GetPath() + m_Filename).c_str());
			return false;
        }

        while ((dirp = readdir(dp)) != NULL)
        {
			auto name = std::string(dirp->d_name);
			if(name != "." && name != "..")
			{
				Entry* e;
				if(dirp->d_type == DT_DIR)
				{
					e = new Directory(name + "/", this);
				}
				else
				{
					e = new File(name, this);
				}
				m_pChildren.push_back(e);
			}
        }
        closedir(dp);
    }
    if(recursive)RecursiveMount();
    m_IsMounted = true;
    return true;
}

bool Directory::DeleteDir()
{
	std::string path = GetPath()+m_Filename;
	int32 result = remove( path.c_str() );
	return result != -1;
}


} // namespace core
} // namespace et
