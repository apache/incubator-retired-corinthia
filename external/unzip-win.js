/* unzip-win.js 1.00                  UTF-8
 *               USE WINDOWS SHELL TO EXTRACT ALL FROM ZIPS
 *
 * Cscript unzip-win.js zip dest
 *   performas an "extract all" of the zip to the dest folder, where
 *
 *     zip is the full path file location of the zip to extract
 *    dest is the full path of the folder to which extraction occurs
 *
 * This is a helper script designed to be used with a larger script or
 * batch file that provides clean parameters and usage.
 *
 * The present script is adapted from the solution by Greg Zakharov at
 * <http://stackoverflow.com/a/19711019/33810> on 2013-10-31.
 *
 * TODO
 *   It is a little startling when the Windows copying-files animation
 *   pops up when one of these runs long enough for its visibility.
 *   Find a way to inhibit that, if possible.
 */


try
{ var  zip = WScript.Arguments(0);
  var dest = WScript.Arguments(1);

  with (new ActiveXObject('Scripting.FileSystemObject'))
  {
    if (!FolderExists(dest)) CreateFolder(dest);
    with (new ActiveXObject('Shell.Application'))
    {
      NameSpace(GetFolder(dest).Path)
         .CopyHere(Namespace(GetFile(zip).Path).Items());
    }
  }
}
catch (e)
{
  WScript.echo(e.message);
}

/* 1.00 2015-01-02 Complete Adaptation for Corinthia Externals Extraction
 */

/*                   *** end of unzip-win.js ***                           */

