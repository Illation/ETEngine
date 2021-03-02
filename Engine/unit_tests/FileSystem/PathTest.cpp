#include <EtFramework/stdafx.h>

#include <catch2/catch.hpp>

#include <mainTesting.h>

#include <EtCore/FileSystem2/Path.h>


using namespace et;


TEST_CASE("path_create_absolute_volume", "[filesystem]")
{
#ifdef ET_PLATFORM_WIN

	core::Path const p1("C:\\some\\./dir\\name.ext");

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

	REQUIRE(p1.HasParentPath());

	REQUIRE(p1.GetName() == "name.ext");

	REQUIRE(p1.GetRawName() == "name");
	REQUIRE(p1.GetRawNameId() == "name"_hash);

	REQUIRE(p1.GetExtension() == "ext");
	REQUIRE(p1.GetExtensionId() == "ext"_hash);
	REQUIRE(p1.IsFile() == true);
}

TEST_CASE("path_create_absolute", "[filesystem]")
{
	core::Path const p1("/some/./dir/name.ext");

	REQUIRE(p1.Get() == "/some/dir/name.ext");
	REQUIRE(p1.GetId() == "/some/dir/name.ext"_hash);

	REQUIRE(p1.GetParentPath() == "/some/dir/");
	REQUIRE(p1.HasParentPath());

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
	REQUIRE(p1.HasParentPath());

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
	REQUIRE(p1.HasParentPath());

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
	core::Path const p1("some/other/../dir/");

	REQUIRE(p1.Get() == "some/dir/");
	REQUIRE(p1.GetId() == "some/dir/"_hash);

	REQUIRE(p1.GetParentPath() == "some/");
	REQUIRE(p1.HasParentPath());

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
	REQUIRE(!p1.HasParentPath());

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
	REQUIRE(!p1.HasParentPath());

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

	REQUIRE(p1.Get() == "./");
	REQUIRE(p1.GetId() == "./"_hash);

	REQUIRE(p1.GetParentPath() == "");
	REQUIRE(!p1.HasParentPath());

	REQUIRE(p1.GetVolume() == "");
	REQUIRE(p1.GetVolumeId().IsEmpty());

	REQUIRE(p1.GetType() == core::Path::E_Type::Relative);

	REQUIRE(p1.GetName() == "./");

	REQUIRE(p1.GetRawName() == "./");
	REQUIRE(p1.GetRawNameId() == "./"_hash);

	REQUIRE(p1.GetExtension() == "");
	REQUIRE(p1.GetExtensionId().IsEmpty());
	REQUIRE(p1.IsFile() == false);
}

TEST_CASE("path_create_from_base", "[filesystem]")
{
	core::Path const p1("/some/.");
	core::Path const p2(p1, "dir/name.ext");

	REQUIRE(p2.Get() == "/some/dir/name.ext");
	REQUIRE(p2.GetId() == "/some/dir/name.ext"_hash);

	REQUIRE(p2.GetParentPath() == "/some/dir/");
	REQUIRE(p2.HasParentPath());

	REQUIRE(p2.GetVolume() == "/");
	REQUIRE(p2.GetVolumeId() == "/"_hash);

	REQUIRE(p2.GetType() == core::Path::E_Type::Absolute);

	REQUIRE(p2.GetName() == "name.ext");

	REQUIRE(p2.GetRawName() == "name");
	REQUIRE(p2.GetRawNameId() == "name"_hash);

	REQUIRE(p2.GetExtension() == "ext");
	REQUIRE(p2.GetExtensionId() == "ext"_hash);
	REQUIRE(p2.IsFile() == true);

	core::Path const p3(p1, "dir/");
	REQUIRE(p3.Get() == "/some/dir/");
	REQUIRE(p3.GetId() == "/some/dir/"_hash);

	REQUIRE(p3.GetParentPath() == "/some/");
	REQUIRE(p3.HasParentPath());

	REQUIRE(p3.GetVolume() == "/");
	REQUIRE(p3.GetVolumeId() == "/"_hash);

	REQUIRE(p3.GetType() == core::Path::E_Type::Absolute);

	REQUIRE(p3.GetName() == "dir/");

	REQUIRE(p3.GetRawName() == "dir/");
	REQUIRE(p3.GetRawNameId() == "dir/"_hash);

	REQUIRE(p3.GetExtension() == "");
	REQUIRE(p3.GetExtensionId().IsEmpty());
	REQUIRE(p3.IsFile() == false);

	core::Path const p4(core::Path(""), "some/dir/");

	REQUIRE(p4.Get() == "some/dir/");
	REQUIRE(p4.GetId() == "some/dir/"_hash);

	REQUIRE(p4.GetParentPath() == "some/");
	REQUIRE(p4.HasParentPath());

	REQUIRE(p4.GetVolume() == "");
	REQUIRE(p4.GetVolumeId().IsEmpty());

	REQUIRE(p4.GetType() == core::Path::E_Type::Relative);

	REQUIRE(p4.GetName() == "dir/");

	REQUIRE(p4.GetRawName() == "dir/");
	REQUIRE(p4.GetRawNameId() == "dir/"_hash);

	REQUIRE(p4.GetExtension() == "");
	REQUIRE(p4.GetExtensionId().IsEmpty());
	REQUIRE(p4.IsFile() == false);

	core::Path const p5(core::Path("some/dir/"), ".");
	core::Path const p6(core::Path("some/dir/"), "./");
	core::Path const p7(core::Path("some/dir/"), "");
	core::Path const p8(core::Path("some/dir/and/"), "..");
	core::Path const p9(core::Path("some/dir/and/"), "../");

	REQUIRE(p5.Get() == "some/dir/");

	REQUIRE(p5.GetId() == "some/dir/"_hash);
	REQUIRE(p5.GetId() == p6.GetId());
	REQUIRE(p5.GetId() == p7.GetId());
	REQUIRE(p5.GetId() == p8.GetId());
	REQUIRE(p5.GetId() == p9.GetId());

	REQUIRE(p5.GetParentPath() == "some/");
	REQUIRE(p5.HasParentPath());

	REQUIRE(p5.GetVolume() == "");
	REQUIRE(p5.GetVolumeId().IsEmpty());

	REQUIRE(p5.GetType() == core::Path::E_Type::Relative);

	REQUIRE(p5.GetName() == "dir/");

	REQUIRE(p5.GetRawName() == "dir/");
	REQUIRE(p5.GetRawNameId() == "dir/"_hash);

	REQUIRE(p5.GetExtension() == "");
	REQUIRE(p5.GetExtensionId().IsEmpty());
	REQUIRE(p5.IsFile() == false);

	core::Path const p10(core::Path("/some/dir/and/"), "../name.ext");

	REQUIRE(p10.Get() == "/some/dir/name.ext");
	REQUIRE(p10.GetId() == "/some/dir/name.ext"_hash);

	REQUIRE(p10.GetParentPath() == "/some/dir/");
	REQUIRE(p10.HasParentPath());

	REQUIRE(p10.GetVolume() == "/");
	REQUIRE(p10.GetVolumeId() == "/"_hash);

	REQUIRE(p10.GetType() == core::Path::E_Type::Absolute);

	REQUIRE(p10.GetName() == "name.ext");

	REQUIRE(p10.GetRawName() == "name");
	REQUIRE(p10.GetRawNameId() == "name"_hash);

	REQUIRE(p10.GetExtension() == "ext");
	REQUIRE(p10.GetExtensionId() == "ext"_hash);
	REQUIRE(p10.IsFile() == true);
}

TEST_CASE("path_create_from_combination", "[filesystem]")
{
	core::Path const p1("/some/.");
	core::Path const p2(p1, core::Path("dir/name.ext"));

	REQUIRE(p2.Get() == "/some/dir/name.ext");
	REQUIRE(p2.GetId() == "/some/dir/name.ext"_hash);

	REQUIRE(p2.GetParentPath() == "/some/dir/");
	REQUIRE(p2.HasParentPath());

	REQUIRE(p2.GetVolume() == "/");
	REQUIRE(p2.GetVolumeId() == "/"_hash);

	REQUIRE(p2.GetType() == core::Path::E_Type::Absolute);

	REQUIRE(p2.GetName() == "name.ext");

	REQUIRE(p2.GetRawName() == "name");
	REQUIRE(p2.GetRawNameId() == "name"_hash);

	REQUIRE(p2.GetExtension() == "ext");
	REQUIRE(p2.GetExtensionId() == "ext"_hash);
	REQUIRE(p2.IsFile() == true);

	core::Path const p3(p1, core::Path("dir/"));
	REQUIRE(p3.Get() == "/some/dir/");
	REQUIRE(p3.GetId() == "/some/dir/"_hash);

	REQUIRE(p3.GetParentPath() == "/some/");
	REQUIRE(p3.HasParentPath());

	REQUIRE(p3.GetVolume() == "/");
	REQUIRE(p3.GetVolumeId() == "/"_hash);

	REQUIRE(p3.GetType() == core::Path::E_Type::Absolute);

	REQUIRE(p3.GetName() == "dir/");

	REQUIRE(p3.GetRawName() == "dir/");
	REQUIRE(p3.GetRawNameId() == "dir/"_hash);

	REQUIRE(p3.GetExtension() == "");
	REQUIRE(p3.GetExtensionId().IsEmpty());
	REQUIRE(p3.IsFile() == false);

	core::Path const p4(core::Path(""), core::Path("some/dir/"));

	REQUIRE(p4.Get() == "some/dir/");
	REQUIRE(p4.GetId() == "some/dir/"_hash);

	REQUIRE(p4.GetParentPath() == "some/");
	REQUIRE(p4.HasParentPath());

	REQUIRE(p4.GetVolume() == "");
	REQUIRE(p4.GetVolumeId().IsEmpty());

	REQUIRE(p4.GetType() == core::Path::E_Type::Relative);

	REQUIRE(p4.GetName() == "dir/");

	REQUIRE(p4.GetRawName() == "dir/");
	REQUIRE(p4.GetRawNameId() == "dir/"_hash);

	REQUIRE(p4.GetExtension() == "");
	REQUIRE(p4.GetExtensionId().IsEmpty());
	REQUIRE(p4.IsFile() == false);

	core::Path const p5(core::Path("some/dir/"), core::Path("."));
	core::Path const p6(core::Path("some/dir/"), core::Path("./"));
	core::Path const p7(core::Path("some/dir/"), core::Path(""));
	core::Path const p8(core::Path("some/dir/and/"), core::Path(".."));
	core::Path const p9(core::Path("some/dir/and/"), core::Path("../"));

	REQUIRE(p5.Get() == "some/dir/");

	REQUIRE(p5.GetId() == "some/dir/"_hash);
	REQUIRE(p5.GetId() == p6.GetId());
	REQUIRE(p5.GetId() == p7.GetId());
	REQUIRE(p5.GetId() == p8.GetId());
	REQUIRE(p5.GetId() == p9.GetId());

	REQUIRE(p5.GetParentPath() == "some/");
	REQUIRE(p5.HasParentPath());

	REQUIRE(p5.GetVolume() == "");
	REQUIRE(p5.GetVolumeId().IsEmpty());

	REQUIRE(p5.GetType() == core::Path::E_Type::Relative);

	REQUIRE(p5.GetName() == "dir/");

	REQUIRE(p5.GetRawName() == "dir/");
	REQUIRE(p5.GetRawNameId() == "dir/"_hash);

	REQUIRE(p5.GetExtension() == "");
	REQUIRE(p5.GetExtensionId().IsEmpty());
	REQUIRE(p5.IsFile() == false);

	core::Path const p10(core::Path("/some/dir/and/"), core::Path("../name.ext"));

	REQUIRE(p10.Get() == "/some/dir/name.ext");
	REQUIRE(p10.GetId() == "/some/dir/name.ext"_hash);

	REQUIRE(p10.GetParentPath() == "/some/dir/");
	REQUIRE(p10.HasParentPath());

	REQUIRE(p10.GetVolume() == "/");
	REQUIRE(p10.GetVolumeId() == "/"_hash);

	REQUIRE(p10.GetType() == core::Path::E_Type::Absolute);

	REQUIRE(p10.GetName() == "name.ext");

	REQUIRE(p10.GetRawName() == "name");
	REQUIRE(p10.GetRawNameId() == "name"_hash);

	REQUIRE(p10.GetExtension() == "ext");
	REQUIRE(p10.GetExtensionId() == "ext"_hash);
	REQUIRE(p10.IsFile() == true);

	core::Path const p11(core::Path("/some/dir/and/"), core::Path("../"));

	REQUIRE(p11.Get() == "/some/dir/");
	REQUIRE(p11.GetId() == "/some/dir/"_hash);

	REQUIRE(p11.GetParentPath() == "/some/");
	REQUIRE(p11.HasParentPath());

	REQUIRE(p11.GetVolume() == "/");
	REQUIRE(p11.GetVolumeId() == "/"_hash);

	REQUIRE(p11.GetType() == core::Path::E_Type::Absolute);

	REQUIRE(p11.GetName() == "dir/");

	REQUIRE(p11.GetRawName() == "dir/");
	REQUIRE(p11.GetRawNameId() == "dir/"_hash);

	REQUIRE(p11.GetExtension() == "");
	REQUIRE(p11.GetExtensionId().IsEmpty());
	REQUIRE(p11.IsFile() == false);

	core::Path const p12(core::Path("some/dir/"), core::Path("../../../"));

	REQUIRE(p12.Get() == "../");
	REQUIRE(p12.GetId() == "../"_hash);

	REQUIRE(p12.GetVolumeId().IsEmpty());
	REQUIRE(p12.GetType() == core::Path::E_Type::Relative);

	REQUIRE(p12.GetName() == "../");

	REQUIRE(p12.GetRawNameId() == "../"_hash);

	REQUIRE(p12.GetExtension() == "");
	REQUIRE(p12.GetExtensionId().IsEmpty());
	REQUIRE(p12.IsFile() == false);

	core::Path const p13(core::Path("some/dir/"), core::Path("../../../name.ext"));

	REQUIRE(p13.Get() == "../name.ext");
	REQUIRE(p13.GetId() == "../name.ext"_hash);

	REQUIRE(p13.GetVolumeId().IsEmpty());
	REQUIRE(p13.GetType() == core::Path::E_Type::Relative);

	REQUIRE(p13.GetName() == "name.ext");

	REQUIRE(p13.GetRawNameId() == "name"_hash);

	REQUIRE(p13.GetExtension() == "ext");
	REQUIRE(p13.GetExtensionId() == "ext"_hash);
	REQUIRE(p13.IsFile() == true);

	core::Path const p14(core::Path("../some/"), core::Path("../../../name.ext"));
	REQUIRE(p14.Get() == "../../../name.ext");

	core::Path const p15(core::Path("../"), core::Path("../../../"));
	REQUIRE(p15.Get() == "../../../../");

	core::Path const p16(core::Path("../some/"), core::Path("../"));
	REQUIRE(p16.Get() == "../");

	core::Path const p17(core::Path("../some/"), core::Path("../../"));
	REQUIRE(p17.Get() == "../../");

	core::Path const p18(core::Path("../some/dir/"), core::Path("../"));
	REQUIRE(p18.Get() == "../some/");

	core::Path const p19(core::Path("../some/dir/"), core::Path("../name"));
	REQUIRE(p19.Get() == "../some/name");

	core::Path const p20(core::Path("../../../"), core::Path("../"));
	REQUIRE(p20.Get() == "../../../../");
}

TEST_CASE("path_append", "[filesystem]")
{
	core::Path const p1("/some/.");
	core::Path p2(p1);
	p2.Append(core::Path("dir/name.ext"));

	REQUIRE(p2.Get() == "/some/dir/name.ext");
	REQUIRE(p2.GetId() == "/some/dir/name.ext"_hash);

	REQUIRE(p2.GetParentPath() == "/some/dir/");
	REQUIRE(p2.HasParentPath());

	REQUIRE(p2.GetVolume() == "/");
	REQUIRE(p2.GetVolumeId() == "/"_hash);

	core::Path p3(p1);
	p3.Append(core::Path("dir/"));
	REQUIRE(p3.Get() == "/some/dir/");

	core::Path p4(core::Path(""));
	p4.Append(core::Path("some/dir/"));
	REQUIRE(p4.Get() == "some/dir/");

	core::Path p5(core::Path("some/dir/"));
	p5.Append(core::Path("."));
	core::Path p6(core::Path("some/dir/"));
	p6.Append(core::Path("./"));
	core::Path p7(core::Path("some/dir/"));
	p7.Append(core::Path(""));
	core::Path p8(core::Path("some/dir/and/"));
	p8.Append(core::Path(".."));
	core::Path p9(core::Path("some/dir/and/"));
	p9.Append(core::Path("../"));

	REQUIRE(p5.Get() == "some/dir/");

	REQUIRE(p5.GetId() == "some/dir/"_hash);
	REQUIRE(p5.GetId() == p6.GetId());
	REQUIRE(p5.GetId() == p7.GetId());
	REQUIRE(p5.GetId() == p8.GetId());
	REQUIRE(p5.GetId() == p9.GetId());

	core::Path p10(core::Path("/some/dir/and/"));
	p10.Append(core::Path("../name.ext"));
	REQUIRE(p10.Get() == "/some/dir/name.ext");

	core::Path p11(core::Path("/some/dir/and/"));
	p11.Append(core::Path("../"));
	REQUIRE(p11.Get() == "/some/dir/");

	core::Path p12(core::Path("some/dir/"));
	p12.Append(core::Path("../../../"));
	REQUIRE(p12.Get() == "../");

	core::Path p13(core::Path("some/dir/"));
	p13.Append(core::Path("../../../name.ext"));
	REQUIRE(p13.Get() == "../name.ext");

	core::Path p14(core::Path("../some/"));
	p14.Append(core::Path("../../../name.ext"));
	REQUIRE(p14.Get() == "../../../name.ext");

	core::Path p15(core::Path("../"));
	p15.Append(core::Path("../../../"));
	REQUIRE(p15.Get() == "../../../../");

	core::Path p16(core::Path("../some/"));
	p16.Append(core::Path("../"));
	REQUIRE(p16.Get() == "../");

	core::Path p17(core::Path("../some/"));
	p17.Append(core::Path("../../"));
	REQUIRE(p17.Get() == "../../");

	core::Path p18(core::Path("../some/dir/"));
	p18.Append(core::Path("../"));
	REQUIRE(p18.Get() == "../some/");

	core::Path p19(core::Path("../some/dir/"));
	p19.Append(core::Path("../name"));
	REQUIRE(p19.Get() == "../some/name");

	core::Path p20(core::Path("../../../"));
	p20.Append(core::Path("../"));
	REQUIRE(p20.Get() == "../../../../");
}

TEST_CASE("path_pop_front", "[filesystem]")
{
	core::Path p1("./");
	p1.PopFront();
	REQUIRE(p1.Get() == "./");
	REQUIRE(p1.GetId() == "./"_hash);
	REQUIRE(p1.GetVolumeId().IsEmpty());
	REQUIRE(p1.GetType() == core::Path::E_Type::Relative);
	REQUIRE(p1.GetExtensionId().IsEmpty());

	core::Path p2("/some/");
	p2.PopFront();
	REQUIRE(p2.Get() == "some/");
	REQUIRE(p2.GetId() == "some/"_hash);
	REQUIRE(p2.GetVolumeId().IsEmpty());
	REQUIRE(p2.GetType() == core::Path::E_Type::Relative);
	REQUIRE(p2.GetExtensionId().IsEmpty());

	core::Path p3("some/dir/name.ext");
	p3.PopFront();
	REQUIRE(p3.Get() == "dir/name.ext");
	REQUIRE(p3.GetId() == "dir/name.ext"_hash);
	REQUIRE(p3.GetVolumeId().IsEmpty());
	REQUIRE(p3.GetType() == core::Path::E_Type::Relative);
	REQUIRE(p3.GetExtensionId() == "ext"_hash);

	core::Path p4("name.ext");
	p4.PopFront();
	REQUIRE(p4.Get() == "./");
	REQUIRE(p4.GetId() == "./"_hash);
	REQUIRE(p4.GetVolumeId().IsEmpty());
	REQUIRE(p4.GetType() == core::Path::E_Type::Relative);
	REQUIRE(p4.GetExtensionId().IsEmpty());
}

TEST_CASE("path_pop_back", "[filesystem]")
{
	core::Path p1("./");
	p1.PopBack();
	REQUIRE(p1.Get() == "./");
	REQUIRE(p1.GetId() == "./"_hash);
	REQUIRE(p1.GetVolumeId().IsEmpty());
	REQUIRE(p1.GetType() == core::Path::E_Type::Relative);
	REQUIRE(p1.GetExtensionId().IsEmpty());

	core::Path p2("/some/");
	p2.PopBack();
	REQUIRE(p2.Get() == "/");
	REQUIRE(p2.GetId() == "/"_hash);
	REQUIRE(p2.GetVolumeId() == "/"_hash);
	REQUIRE(p2.GetType() == core::Path::E_Type::Absolute);
	REQUIRE(p2.GetExtensionId().IsEmpty());

	core::Path p3("some/dir/name.ext");
	p3.PopBack();
	REQUIRE(p3.Get() == "some/dir/");
	REQUIRE(p3.GetId() == "some/dir/"_hash);
	REQUIRE(p3.GetVolumeId().IsEmpty());
	REQUIRE(p3.GetType() == core::Path::E_Type::Relative);
	REQUIRE(p3.GetExtensionId().IsEmpty());

	core::Path p4("name.ext");
	p4.PopBack();
	REQUIRE(p4.Get() == "./");
	REQUIRE(p4.GetId() == "./"_hash);
	REQUIRE(p4.GetVolumeId().IsEmpty());
	REQUIRE(p4.GetType() == core::Path::E_Type::Relative);
	REQUIRE(p4.GetExtensionId().IsEmpty());
}

TEST_CASE("path_make_relative", "[filesystem]")
{
	core::Path p1("./");
	p1.MakeRelative(core::Path("./"));
	REQUIRE(p1.Get() == "./");
	REQUIRE(p1.GetId() == "./"_hash);
	REQUIRE(p1.GetVolumeId().IsEmpty());
	REQUIRE(p1.GetType() == core::Path::E_Type::Relative);
	REQUIRE(p1.GetExtensionId().IsEmpty());

	core::Path p2("./");
	p2.MakeRelative(core::Path("some/"));
	REQUIRE(p2.Get() == "some/");
	REQUIRE(p2.GetId() == "some/"_hash);
	REQUIRE(p2.GetVolumeId().IsEmpty());
	REQUIRE(p2.GetType() == core::Path::E_Type::Relative);
	REQUIRE(p2.GetExtensionId().IsEmpty());

	core::Path p3("some/");
	p3.MakeRelative(core::Path("./"));
	REQUIRE(p3.Get() == "some/");
	REQUIRE(p3.GetId() == "some/"_hash);
	REQUIRE(p3.GetVolumeId().IsEmpty());
	REQUIRE(p3.GetType() == core::Path::E_Type::Relative);
	REQUIRE(p3.GetExtensionId().IsEmpty());

	core::Path p4("/some/dir/name.ext");
	p4.MakeRelative(core::Path("/some/dir/"));
	REQUIRE(p4.Get() == "name.ext");
	REQUIRE(p4.GetId() == "name.ext"_hash);
	REQUIRE(p4.GetVolumeId().IsEmpty());
	REQUIRE(p4.GetType() == core::Path::E_Type::Relative);
	REQUIRE(p4.GetExtensionId() == "ext"_hash);

	core::Path p5("/some/");
	p5.MakeRelative(core::Path("/some/dir/"));
	REQUIRE(p5.Get() == "../");
	REQUIRE(p5.GetId() == "../"_hash);
	REQUIRE(p5.GetVolumeId().IsEmpty());
	REQUIRE(p5.GetType() == core::Path::E_Type::Relative);
	REQUIRE(p5.GetExtensionId().IsEmpty());

	core::Path p6("/some/other/name.ext");
	p6.MakeRelative(core::Path("/some/dir/"));
	REQUIRE(p6.Get() == "../other/name.ext");
	REQUIRE(p6.GetId() == "../other/name.ext"_hash);
	REQUIRE(p6.GetVolumeId().IsEmpty());
	REQUIRE(p6.GetType() == core::Path::E_Type::Relative);
	REQUIRE(p6.GetExtensionId() == "ext"_hash);
}
