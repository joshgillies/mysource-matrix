<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ABN 77 084 670 600                                                 |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: file_system.inc,v 1.50 2012/08/30 00:56:51 ewang Exp $
*
*/


/**
* File System Functions
*
* Purpose
*     Generic functions for the file system
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Revision: 1.50 $
* @package Fudge
* @subpackage General
*/


/**
* Takes a filename and extracts the extension
* If the parameter passed is not a string then it returns an empty string
*
* @param string	$filename	the file whose extension to get
*
* @return string
* @access public
*/
function get_file_type($filename)
{
	if (is_string($filename)) {
		return strtolower(  substr( strrchr($filename,'.') , 1)  );
	} else {
		return '';
	}

}//end get_file_type()


/**
* Takes a filename and returns true if it is considdered and image
*
* @param string	$filename	the file to check
*
* @return string
* @access public
*/
function is_image($filename)
{

	switch (get_file_type($filename)) {
		case 'jpg'  :
		case 'jpeg' :
		case 'gif'  :
		case 'png'  :
			return TRUE;
		default :
			return FALSE;
	} //end switch

}//end is_image()


/**
* Reads a file into a string
*
* @param string	$filename	the filename whose contents to return
*
* @return mixed string|boolean
* @access public
*/
function file_to_string($filename)
{
	if (!is_readable($filename)) return FALSE;
	return file_get_contents($filename);

}//end file_to_string()


/**
* Writes a string to a file.. overwrites current file
*
* @param string	$string		the string to write
* @param string	$filename	the filename to write to
*
* @return boolean
* @access public
*/
function string_to_file($string, $filename)
{
	if (!is_string($filename) || empty($filename)) {
		return FALSE;
	}

	// this function writes into the files only in the existing directories
	// if directory doe snot exist then it returns false

	if (!is_dir(dirname($filename))) return FALSE;

	$tmp_filename = tempnam(dirname($filename), __FUNCTION__);
	if (!$f = fopen($tmp_filename, 'wb')) {
		trigger_error('Unable to open temporary file ('.$tmp_filename.') for writing, unable to write '.$filename, E_USER_WARNING);
		return FALSE;
	}
	$s = fputs($f, $string);
	fclose($f);
	if ($s === FALSE) {
		trigger_error('Unable to write '.$filename, E_USER_WARNING);
		unlink($tmp_filename);
		return FALSE;
	}

	$om = umask(0000);
	chmod($tmp_filename, 0664);
	umask($om);

	// if the file being renamed to exists, have to delete it because winnt doesn't overwrite on
	// rename
	if ((strtolower(PHP_OS) == 'winnt') || (strtolower(PHP_OS) == 'win32')) {
		// make sure we're using the latest info
		clearstatcache();

		if (file_exists($filename)) unlink($filename);
	}

	if (rename($tmp_filename, $filename)) {
		return TRUE;
	} else {
		unlink($tmp_filename);
		return FALSE;
	}

}//end string_to_file()


/**
* Writes an array to a file. The file can be required/included to
* retrieve the array again
*
* @param array	$value		the array to write
* @param string	$var_name	the name of the variable when included
* @param string	$filename	the filename to write to
*
* @return boolean
* @access public
*/
function array_to_file($value, $var_name, $filename)
{
	if (!is_array($value)) return FALSE;
	$string = '<?php $'.$var_name.' = '.var_export($value, TRUE).'; ?>';
	return string_to_file($string, $filename);

}//end array_to_file()


/**
* Takes an int representing the number of bytes and returns a easy to read string
*
* @param int	$size	the size in bytes
*
* @return string
* @access public
*/
function easy_filesize($size)
{
	if ($size < 1024) {
		return $size.' Bytes';
	} else if ($size < 1048576) {
		return sprintf('%.1f KB', $size/1024.0);
	} else if ($size < 1073741824) {
		return sprintf('%.1f MB',($size/1024.0)/1024.0);
	} else {
		return sprintf('%.1f GB',(($size/1024.0)/1024.0)/1024.0);
	}

}//end easy_filesize()


/**
* Same as increment_name(), but preserves a filename extension
*
* @param string	$name	the original name of the file
* @param string	$spacer	the char(s) to put between the end of the name and the numeral
*
* @return string
* @access public
* @see Fudge::general::increment_name()
*/
function increment_filename($name,$spacer='')
{
	require_once dirname(__FILE__).'/../general/general.inc';
	if (strpos($name,'.') !== FALSE) {
		$ext = get_file_type($name);
		return increment_name(substr($name,0,-strlen($ext)-1),$spacer).'.'.$ext;
	} else {
		return increment_name($name, $spacer);
	}

}//end increment_filename()


/**
* Function to return an array of files in a directory
*
* @param string		$dir		The directory you want to list files for.
*								If you pass in a filename, it will take the directory of that
*								file name and restrict the returned files to the type of file you passed in.
* @param boolean	$fullpath	whether to return the full path to the file or just the filenames themselves
*
* @return array
* @access public
*/
function list_files($dir, $fullpath=FALSE)
{
	if (!$dir) return FALSE;
	if (is_dir($dir)) {
		$restrict = FALSE;
	} else {
		$restrict = basename(str_replace('*','',$dir)); // if we pass in *.inc, check for .inc
		$dir = dirname($dir);
	}

	if (!is_dir($dir)) return Array();

	$files = Array();
	if ($handle = opendir($dir)) {
		while (($file = readdir($handle)) !== FALSE) {
			if ($file == '.' || $file == '..') {
				continue;
			}
			if (is_file($dir.'/'.$file)) {
				if ($restrict && (preg_match("%$restrict%", $file))) {
					$files[] = ($fullpath) ? $dir.'/'.$file : $file;
				}
				if (!$restrict) {
					$files[] = ($fullpath) ? $dir.'/'.$file : $file;
				}
			}
		}
		closedir($handle);
	}
	return $files;

}//end list_files()


/**
* Function to return an array of sub directories in a directory
*
* @param string		$dir			the directory you want to list sub directories for
* @param boolean	$fullpath		whether to return the full path to the direcotry or just the names themselves
* @param array		$ignore_list	list of directories which will be ignored
* @param boolean	$recursive		if true the function will recursively find every subfolders and $fullpath will return
*
* @return array
* @access public
*/
function list_dirs($dir, $fullpath=FALSE, $ignore_list=Array(), $recursive=FALSE)
{
	if (!is_dir($dir)) return Array();

	// if recursive we will be using fullpath
	if ($recursive) $fullpath = TRUE;

	array_push($ignore_list, '.', '..');

	$dirs = Array();
	if ($handle = opendir($dir)) {
		while (($subdir = readdir($handle)) !== FALSE) {
			if (!is_dir($dir.'/'.$subdir) || in_array($subdir, $ignore_list)) {
				continue;
			}
			$dirs[] = ($fullpath) ? $dir.'/'.$subdir : $subdir;
			if ($recursive) {
				$dirs = array_merge($dirs, list_dirs($dir.'/'.$subdir, $fullpath, $ignore_list, TRUE));
			}
		}
		closedir($handle);
	}
	return $dirs;

}//end list_dirs()


/**
* Creates a directory, including any parent directories needed to get to that directory
* TODO: This function should not allow integers. It is the responsibility of the calling function
* to typecast the argument to a String before calling this function.
*
* @param string		$path			The path to create
* @param boolean	$show_warning	If true, trigger a user warning for logs.
*
* @return boolean
* @access public
*/
function create_directory($path, $show_warning=TRUE)
{
	if ((!is_string($path) &&(!is_int($path))) || empty($path)) {
		return FALSE;
	}
	if (is_dir($path)) return TRUE;

	$old_umask = umask(0);
	if (!@mkdir($path, 0775, TRUE)) {
		if ($show_warning) {
			trigger_error('Unable to create directory: '.$path, E_USER_WARNING);
		}

		clearstatcache();
		return FALSE;
	}

	umask($old_umask);
	clearstatcache();
	return TRUE;

}//end create_directory()


/**
* Recursively delete the contents of a directory without system calls
*
* @param string	$path	the path to delete
*
* @return boolean
* @access public
*/
function delete_directory($path)
{
	$path  = rtrim($path, '/');
	if (!is_dir($path)) {
		trigger_error($path.' is not a directory (for deletion).', E_USER_WARNING);
		clearstatcache();
		return FALSE;
	}
	if (clear_directory($path)) {
		if (rmdir($path)) {
			clearstatcache();
			return TRUE;
		} else {
			trigger_error('Unable to delete dir: '.$path, E_USER_WARNING);
			clearstatcache();
			return FALSE;
		}
	}
	clearstatcache();
	return FALSE;

}//end delete_directory()


/**
* Deletes all contents of a directory without system calls
*
* @param string	$path	the directory whose contents to delete
* @param array	$file_excl	filenames in the directory you don't want to delete	
*
* @return boolean
* @access public
*/
function clear_directory($path, $file_excl = Array())
{
	$path  = rtrim($path, '/');
	if (!is_dir($path)) {
		trigger_error($path.' is not a directory (for clearing).');
		clearstatcache();
		return FALSE;
	}
	$dir = opendir($path);
	while (FALSE !== ($filename = readdir($dir))) {
		if ($filename == '.' || $filename == '..' || in_array($filename, $file_excl)) {
			continue;
		}
		$filename = $path.'/'.$filename;
		if (is_dir($filename)) {
			delete_directory($filename);
		} else {
			if (!unlink($filename)) {
				trigger_error('Unable to delete: '.$filename, E_USER_WARNING);
				clearstatcache();
				return FALSE;
			}
		}
	}
	closedir($dir);
	clearstatcache();
	return TRUE;

}//end clear_directory()


/**
* Recursively copies the contents of a directory without system calls
*
* @param string	$path		the directory to copy
* @param string	$new_path	the parh name to copy to
*
* @return boolean
* @access public
*/
function copy_directory($path, $new_path)
{
	$path     = rtrim($path, '/');
	$new_path = rtrim($new_path, '/');
	if (!is_dir($path)) {
		trigger_error($path.' is not a directory (for copying.)');
		clearstatcache();
		return FALSE;
	}
	if (file_exists($new_path)) {
		if (!is_dir($new_path)) {
			trigger_error($new_path.' exists but is not a directory (for copying into).');
			clearstatcache();
			return FALSE;
		}
	} else {
		if (!create_directory($new_path)) {
			clearstatcache();
			return FALSE;
		}
	}
	// Copy the contents
	$dir = opendir($path);
	while (FALSE !== ($filename = readdir($dir))) {
		if ($filename == '.' || $filename == '..') {
			continue;
		}
		$full_filename = $path.'/'.$filename;
		if (is_dir($full_filename)) {
			if (!copy_directory($full_filename, $new_path.'/'.$filename)) {
				clearstatcache();
				return FALSE;
			}
		} else {
			if (!copy($full_filename, $new_path.'/'.$filename)) {
				trigger_error('Unable to copy: '.$full_filename.' --> '.$new_path.'/'.$filename);
				clearstatcache();
				return FALSE;
			}
		}
	}
	closedir($dir);
	clearstatcache();
	return TRUE;

}//end copy_directory()


/**
* Copies a file, creates any parent directories needed first
*
* @param string	$from	path of source file
* @param string	$to		path of destination file
*
* @return boolean
* @access public
*/
function copy_file($from, $to)
{
	if (!create_directory(dirname($to))) return FALSE;

	if (!copy($from, $to)) return FALSE;
	return TRUE;

}//end copy_file()


/**
* Moves a file, creates any parent directories needed first
*
* @param string	$from	path of source file
* @param string	$to		path of destination file
*
* @return boolean
* @access public
*/
function move_file($from, $to)
{
	if (!copy_file($from, $to)) return FALSE;
	if (!unlink($from)) return FALSE;
	return TRUE;

}//end move_file()


/**
* Truncate a file 
*
* @param string	$file	path of the file
*
* @return boolean
* @access public
*/
function truncate_file($file)
{
	$fp = fopen($file, 'r+');
	if ($fp) {
		if (!ftruncate($fp, 0)) return FALSE;
		fclose($fp);
		return TRUE;
	}//end if
	return FALSE;

}//end truncate_file()


/**
* Gets last N lines from a file as an array (last line last)
* Ignores empty lines
*
* NOTE: Only works properly if line_ending is a single character
*
* @param string	$filepath		path of file to get lines from
* @param int	$max_lines		number of lines to get
* @param string	$line_ending	end of line character
*
* @return array
* @access public
*/
function get_last_lines_from_file($filepath, $max_lines=10, $line_ending="\n")
{
	// type verification
	if (!is_string($filepath) || !is_int($max_lines) || !is_string($line_ending)) {
		return Array();
	}

	$result = Array();
	$max_lines = is_int($max_lines) ? $max_lines : 10;

	if (!file_exists($filepath) || !is_readable($filepath)) {
		return FALSE;
	}

	if (!$handle = fopen($filepath, 'r')) return FALSE;

	// move pointer before the last character
	fseek($handle, -1, SEEK_END);
	$pos = ftell($handle);
	if ($pos === FALSE) return FALSE;

	$chars = 0;
	$current_word = NULL;
	$lines = Array();
	$line_count = 0;
	$ok = TRUE;

	do {
		$char = fgetc($handle);
		if (0 != fseek($handle, -2, SEEK_CUR)) $ok = FALSE;

		if ($char == $line_ending) {
			if (is_null($current_word)) {
				continue;
			} else {
				array_unshift($lines, $current_word);
				$line_count++;
				$current_word = NULL;
			}

		} else if ($char === FALSE) {
			$ok = FALSE;
			continue;

		} else {
			$current_word = $char.$current_word;
		}

		$chars++;
	} while ($max_lines > $line_count && $ok);

	if (!is_null($current_word) && $max_lines > $line_count) {
		array_unshift($lines, $current_word);
	}

	fclose($handle);

	return $lines;

}//end get_last_lines_from_file()


?>
