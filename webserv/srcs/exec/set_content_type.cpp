#include <string>
#include <iostream>

std::string set_content_type(std::string ext)
{
		if (ext == ".aac")
			return ("audio/aac");
		if (ext == ".abw")
			return ("application/x-abiword");
		if (ext == ".arc")
			return ("application/x-freearc");
		if (ext == ".avif")
			return ("image/avif");
		if (ext == ".avi")
			return ("video/x-msvideo");
		if (ext == ".azw")
			return ("application/vnd.amazon.ebook");
		if (ext == ".bin")
			return ("application/octet-stream");
		if (ext == ".bmp")
			return ("image/bmp");
		if (ext == ".bz")
			return ("application/x-bzip");
		if (ext == ".bz2")
			return ("application/x-bzip2");
		if (ext == ".cda")
			return ("application/x-cdf");
		if (ext == ".csh")
			return ("application/x-csh");
		if (ext == ".css")
			return ("text/css");
		if (ext == ".cvs")
			return ("text/cvs");
		if (ext == ".doc")
			return ("application/msword");
		if (ext == ".docx")
			return ("application/vnd.openxmlformats-officedocument.wordprocessingml.document");
		if (ext == ".eot")
			return ("application/vnd.ms-fontobject");
		if (ext == ".epub")
			return ("application/epub+zip");
		if (ext == ".gz")
			return ("application/gzip");
		if (ext == ".gif")
			return ("image/gif");
		if (ext == ".htm" || ext == ".html")
			return ("text/html");
		if (ext == ".ico")
			return ("image/vnd.microsoft.icon");
		if (ext == ".ics")
			return ("text/calendar");
		if (ext == ".jar")
			return ("application/java-archive");
		if (ext == ".jpg" || ext == ".jpeg")
			return ("image/jpeg");
		if (ext == ".js")
			return ("text/javascript");
		if (ext == ".json")
			return ("application/json");
		if (ext == ".jsonld")
			return ("application/ld+json");
		if (ext == ".mid" || ext == ".midi")
			return ("audio/midi");
		if (ext == ".mjs")
			return ("text/javascript");
		if (ext == ".mp3")
			return ("audio/mpeg");
		if (ext == ".mp4")
			return ("video/mp4");
		if (ext == ".mpeg")
			return ("video/mpeg");
		if (ext == ".mpkg")
			return ("application/vnd.apple.installer+xml");
		if (ext == ".odp")
			return ("application/vnd.oasis.opendocument.presentation");
		if (ext == ".ods")
			return ("application/vnd.oasis.opendocument.spreadsheet");
		if (ext == ".odt")
			return ("application/vnd.oasis.opendocument.text");
		if (ext == ".oga")
			return ("audio/ogg");
		if (ext == ".ogv")
			return ("video/ogg");
		if (ext == ".ogx")
			return ("application/ogg");
		if (ext == ".opus")
			return ("audio/opus");
		if (ext == ".otf")
			return ("font/otf");
		if (ext == ".png")
			return ("image/png");
		if (ext == ".pdf")
			return ("application/pdf");
		if (ext == ".php")
			return ("text/x-php");
		if (ext == ".ppt")
			return ("application/vnd.ms-powerpoint");
		if (ext == ".pptx")
			return ("application/vnd.openxmlformats-officedocument.presentationml.presentation");
		if (ext == ".rar")
			return ("application/vnd.rar");
		if (ext == ".rtf")
			return ("application/rtf");
		if (ext == ".sh")
			return ("application/x-sh");
		if (ext == ".svg")
			return ("image/svg+xml");
		if (ext == ".tar")
			return ("application/x-tar");
		if (ext == ".tif" || ext == ".tiff")
			return ("image/tiff");
		if (ext == ".ts")
			return ("video/mp2t");
		if (ext == ".ttf")
			return ("font/ttf");
		if (ext == ".txt")
			return ("text/plain");
		if (ext == ".vsd")
			return ("application/vnd.visio");
		if (ext == ".waw")
			return ("audio/wav");
		if (ext == ".weba")
			return ("audio/webm");
		if (ext == ".webm")
			return ("video/webm");
		if (ext == ".webp")
			return ("image/webp");
		if (ext == ".woff")
			return ("font/woff");
		if (ext == ".woff2")
			return ("font/woff2");
		if (ext == ".xhtml")
			return ("application/xhtml+xml");
		if (ext == ".xls")
			return ("application/vnd.ms-excel");
		if (ext == ".xlsx")
			return ("application/vnd.openxmlformats-officedocument.spreadsheetml.sheet");
		if (ext == ".xml")
			return ("application/xml");
		if (ext == ".xpm")
			return ("image/x-xpixmap");
		if (ext == ".xul")
			return ("application/vnd.mozilla.xul+xml");
		if (ext == ".zip")
			return ("application/zip");
		if (ext == ".7z")
			return ("application/x-7z-compressed");
		if (ext == ".o")
			return ("application/x-object");
		if (ext == ".c")
			return ("text/x-csrc");
		if (ext == ".cpp" || ext == ".c++" || ext == "cxx" || ext == "cc")
			return ("text/x-c++src");
		if (ext == ".h")
			return ("text/x-chdr");
		if (ext == ".hpp"|| ext == "hpp" || ext == "hxx" || ext == "hh")
			return ("text/x-c++hdr");
		if (ext == ".diff" || ext == ".patch")
			return ("text/x-diff");
		if (ext == ".xcf")
			return ("application/x-xcf");
		if (ext == ".java")
			return ("text/x-java");
		if (ext == ".apk")
			return ("application/vnd.android.package-archive");
		return ("application/octet-stream");
}
