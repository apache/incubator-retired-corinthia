/* wget-win.js                        UTF-8
 * Windows JScript for downloading from the URL in the first argument
 * to the file in the second argument.
 *
 * Adapted from wget.js at <http://superuser.com/a/536400/81303> created by
 * unregistered user190042 and last edited 2013-01-15T08:48.
 *
 * This script is designed to be used from the fetch_downloads.bat file
 * That should be nearby in the same directory.
 *
 * With   Cscript wget-win.js url file
 *
 *            url is the absolute URL of the file to download
 *           file is the full-path file address to store the download at
 */

var WinHttpReq = new ActiveXObject("WinHttp.WinHttpRequest.5.1");
WinHttpReq.Open("GET", WScript.Arguments(0), /*async=*/false);
WinHttpReq.Send();

/* All files are downloaded as binary */
BinStream = new ActiveXObject("ADODB.Stream");
BinStream.Type = 1;
BinStream.Open();
BinStream.Write(WinHttpReq.ResponseBody);
BinStream.SaveToFile(WScript.Arguments(1));

/*                      *** end of wget-win.js ***                         */


