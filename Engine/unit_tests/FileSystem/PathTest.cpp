#include <EtFramework/stdafx.h>

#include <catch2/catch.hpp>

#include <mainTesting.h>

#include <EtCore/FileSystem2/Path.h>


using namespace et;


TEST_CASE("path_create_absolute_volume", "[filesystem]")
{
#ifdef ET_PLATFORM_WIN

	core::Path const p1("C:\\some\\dir\\name.ext");

	REQUIRE(p1.Get() == "C:/some/dir/name.ext");
	REQUIRE(p1.GetId() == "C:/some/dir/name.ext"_hash);

	REQUIRE(p1.GetParentPath() == "C:/some/dir/");

	REQUIRE(p1.GetVolume() == "C:/");
	REQUIRE(p1.GetVolumeId() == "C:/"_hash);

	REQUIRE(p1.GetType() == core::Path::E_Type::AbsoluteVolume);

#else

	core::Path const p1("~some/dir/name.ext");

	REQUIRE(p1.Get() == "~some/dir/name.ext");
	REQUIRE(p1.GetId() == "~some/dir/name.ext"_hash);

	REQUIRE(p1.GetParentPath() == "~some/dir/");

	REQUIRE(p1.GetVolume() == "~");
	REQUIRE(p1.GetVolumeId() == "~"_hash);

	REQUIRE(p1.GetType() == core::Path::E_Type::Absolute);

#endif

	REQUIRE(p1.GetName() == "name.ext");

	REQUIRE(p1.GetRawName() == "name");
	REQUIRE(p1.GetRawNameId() == "name"_hash);

	REQUIRE(p1.GetExtension() == "ext");
	REQUIRE(p1.GetExtensionId() == "ext"_hash);
	REQUIRE(p1.IsFile() == true);
}

TEST_CASE("path_create_absolute", "[filesystem]")
{
	core::Path const p1("/some/dir/name.ext");

	REQUIRE(p1.Get() == "/some/dir/name.ext");
	REQUIRE(p1.GetId() == "/some/dir/name.ext"_hash);

	REQUIRE(p1.GetParentPath() == "/some/dir/");

	REQUIRE(p1.GetVolume() == "/");
	REQUIRE(p1.GetVolumeId() == "/"_hash);

	REQUIRE(p1.GetType() == core::Path::E_Type::Absolute);

	REQUIRE(p1.GetName() == "name.ext");

	REQUIRE(p1.GetRawName() == "name");
	REQUIRE(p1.GetRawNameId() == "name"_hash);

	REQUIRE(p1.GetExtension() == "ext");
	REQUIRE(p1.GetExtensionId() == "ext"_hash);
	REQUIRE(p1.IsFile() == true);
}

TEST_CASE("path_create_relative", "[filesystem]")
{
	core::Path const p1("some/dir/name.ext");

	REQUIRE(p1.Get() == "some/dir/name.ext");
	REQUIRE(p1.GetId() == "some/dir/name.ext"_hash);

	REQUIRE(p1.GetParentPath() == "some/dir/");

	REQUIRE(p1.GetVolume() == "");
	REQUIRE(p1.GetVolumeId().IsEmpty());

	REQUIRE(p1.GetType() == core::Path::E_Type::Relative);

	REQUIRE(p1.GetName() == "name.ext");

	REQUIRE(p1.GetRawName() == "name");
	REQUIRE(p1.GetRawNameId() == "name"_hash);

	REQUIRE(p1.GetExtension() == "ext");
	REQUIRE(p1.GetExtensionId() == "ext"_hash);
	REQUIRE(p1.IsFile() == true);
}

TEST_CASE("path_create_relative_noext", "[filesystem]")
{
	core::Path const p1("some/dir/name");

	REQUIRE(p1.Get() == "some/dir/name");
	REQUIRE(p1.GetId() == "some/dir/name"_hash);

	REQUIRE(p1.GetParentPath() == "some/dir/");

	REQUIRE(p1.GetVolume() == "");
	REQUIRE(p1.GetVolumeId().IsEmpty());

	REQUIRE(p1.GetType() == core::Path::E_Type::Relative);

	REQUIRE(p1.GetName() == "name");

	REQUIRE(p1.GetRawName() == "name");
	REQUIRE(p1.GetRawNameId() == "name"_hash);

	REQUIRE(p1.GetExtension() == "");
	REQUIRE(p1.GetExtensionId().IsEmpty());
	REQUIRE(p1.IsFile() == true);
}

TEST_CASE("path_create_dir_relative", "[filesystem]")
{
	core::Path const p1("some/dir/");

	REQUIRE(p1.Get() == "some/dir/");
	REQUIRE(p1.GetId() == "some/dir/"_hash);

	REQUIRE(p1.GetParentPath() == "some/");

	REQUIRE(p1.GetVolume() == "");
	REQUIRE(p1.GetVolumeId().IsEmpty());

	REQUIRE(p1.GetType() == core::Path::E_Type::Relative);

	REQUIRE(p1.GetName() == "dir/");

	REQUIRE(p1.GetRawName() == "dir/");
	REQUIRE(p1.GetRawNameId() == "dir/"_hash);

	REQUIRE(p1.GetExtension() == "");
	REQUIRE(p1.GetExtensionId().IsEmpty());
	REQUIRE(p1.IsFile() == false);
}

TEST_CASE("path_create_relative_name", "[filesystem]")
{
	core::Path const p1("name.ext");

	REQUIRE(p1.Get() == "name.ext");
	REQUIRE(p1.GetId() == "name.ext"_hash);

	REQUIRE(p1.GetParentPath() == "");

	REQUIRE(p1.GetVolume() == "");
	REQUIRE(p1.GetVolumeId().IsEmpty());

	REQUIRE(p1.GetType() == core::Path::E_Type::Relative);

	REQUIRE(p1.GetName() == "name.ext");

	REQUIRE(p1.GetRawName() == "name");
	REQUIRE(p1.GetRawNameId() == "name"_hash);

	REQUIRE(p1.GetExtension() == "ext");
	REQUIRE(p1.GetExtensionId() == "ext"_hash);
	REQUIRE(p1.IsFile() == true);
}

TEST_CASE("path_create_relative_name_noext", "[filesystem]")
{
	core::Path const p1("name");

	REQUIRE(p1.Get() == "name");
	REQUIRE(p1.GetId() == "name"_hash);

	REQUIRE(p1.GetParentPath() == "");

	REQUIRE(p1.GetVolume() == "");
	REQUIRE(p1.GetVolumeId().IsEmpty());

	REQUIRE(p1.GetType() == core::Path::E_Type::Relative);

	REQUIRE(p1.GetName() == "name");

	REQUIRE(p1.GetRawName() == "name");
	REQUIRE(p1.GetRawNameId() == "name"_hash);

	REQUIRE(p1.GetExtension() == "");
	REQUIRE(p1.GetExtensionId().IsEmpty());
	REQUIRE(p1.IsFile() == true);
}

TEST_CASE("path_create_empty", "[filesystem]")
{
	core::Path const p1("");

	REQUIRE(p1.Get() == "");
	REQUIRE(p1.GetId().IsEmpty());

	REQUIRE(p1.GetParentPath() == "");

	REQUIRE(p1.GetVolume() == "");
	REQUIRE(p1.GetVolumeId().IsEmpty());

	REQUIRE(p1.GetType() == core::Path::E_Type::Invalid);

	REQUIRE(p1.GetName() == "");

	REQUIRE(p1.GetRawName() == "");
	REQUIRE(p1.GetRawNameId().IsEmpty());

	REQUIRE(p1.GetExtension() == "");
	REQUIRE(p1.GetExtensionId().IsEmpty());
	REQUIRE(p1.IsFile() == false);
}
