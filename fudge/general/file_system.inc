<?php

/**
* File System Functions
*
* Purpose
*     Generic functions for the file system
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package Fudge
* @subpackage General
*/


/**
* Takes a filename and extracts the extension
*
* @param string	$filename
*
* @return string
* @access public
*/
function get_file_type($filename) 
{
	return strtolower(  substr( strrchr($filename,'.') , 1)  );
}

/**
* Takes a filename and returns true if it is considdered and image
*
* @param string	$filename
*
* @return string
* @access public
*/
function is_image($filename) 
{

	switch(get_file_type($filename)) {
		case 'jpg'  :
		case 'jpeg' :
		case 'gif'  :
		case 'png'  :
			return true;
		default :
			return false;
	} //end switch

}//end is_image()


/**
* Reads a file into a string
*
* @param string $file    the filename whose contents to return 
*
* @return string
*/
function file_to_string($filename) 
{
	if (!$f = fopen($filename, 'r')) {
		trigger_error('Unable to open file for reading: '.$filename, E_USER_WARNING);
		return false;
	}
	$s = '';
	do {
		$s .= fread($f, 4096);
	} while (!feof($f));
	fclose($f);
	return $s;
}// end file_to_string()

/**
* Writes a string to a file.. overwrites current file
*
* @param string	$string	the string to write
* @param string	$file	the filename to write to
*
* @return string
*/
function string_to_file($string, $filename) 
{
	if (!$f = fopen($filename,'w')) {
		trigger_error('Unable to open file for writing: '.$filename, E_USER_WARNING);
		return false;
	}
	$s = fputs($f, $string);
	fclose($f);
	return true;
}// end string_to_file()


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
	if ($size < 1500)			return $size.' bytes';
	elseif ($size < 1500000)	return sprintf('%.1f kb', $size/1024.0);
	elseif ($size < 1500000000)	return sprintf('%.1f mb',($size/1024.0)/1024.0);
	else						return sprintf('%.1f gb',(($size/1024.0)/1024.0)/1024.0);

}// end easy_filesize()


/**
* Takes a timestamp and returns a human-friendly date/time string
*
* @param string	$then	the timestamp to convert
*
* @return string
* @access public
*/
function readable_datetime($then='') {
	return date('d M Y H:i', $then);
}


/**
* Same as increment_name(), but preserves a filename extension
* 
* @param string	$name	the original name of the file
* @param string $spacer	the char(s) to put between the end of the name and the numeral
*
* @return string
* @access public
* @see Fudge::general::increment_name()
*/
function increment_filename($name,$spacer='') {
	require_once dirname(__FILE__).'../general/general.inc';
	if(strpos($name,'.') !== false) {
		$ext = get_file_type($name);
		return increment_name(substr($name,0,-strlen($ext)-1),$spacer).'.'.$ext;
	} else {
		return increment_name($name, $spacer);
	}
}


/**
* Function to return an array of files in a directory.
*
* @param string	dir			The directory you want to list files for. If you pass in a filename, it will take the directory of that file name and restrict the returned files to the type of file you passed in.
* @param bool	fullpath	whether to return the full path to the file or just the filenames themselves
*
* @returns array
* @access public
*/
function list_files($dir='', $fullpath=false) 
{
	if (!$dir) return false;
	if (is_dir($dir)) {
		$restrict = false;
	} else {
		$restrict = basename(str_replace('*','',$dir));	// if we pass in *.inc, check for .inc
		$dir = dirname($dir);
	}

	if (!is_dir($dir)) return Array();
	
	$files = array();
	if ($handle = opendir($dir)) {
		while (($file = readdir($handle)) !== false) {
			if ($file == '.' || $file == '..') continue;
			if (is_file($dir.'/'.$file)) {
				if ($restrict && (preg_match('%$restrict%', '$file'))) {
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
}// end list_files()

/**
* Creates a directory, including any parent directories needed to get to that directory
*
* @param string	$path	the path to create
*
* @return bool
* @access public
*/
function create_directory($path)
{
	$branches = explode('/', $path);
	$trunk = '';
	$old_umask = umask(0);
	for($i = 0, $count = count($branches); $i < $count; $i++) {
		$trunk .= $branches[$i].'/';
		if(file_exists($trunk)) continue;
		if (!mkdir(substr($trunk,0,-1), 0775)) {
			trigger_error('Unable to create directory: '.$trunk, E_USER_WARNING);
			clearstatcache();
			return false;
		}
	}
	umask($old_umask);
	clearstatcache();
	return true;
}// end create_directory()



/**
* Recursively delete the contents of a directory without system calls 
*
* @param string	$path	the path to delete
*
* @return bool
* @access public
*/
function delete_directory($path) 
{
	$path  = ereg_replace('/$','',$path);
	if (!is_dir($path)) {
		trigger_error($path.' is not a directory (for deletion).', E_USER_WARNING);
		clearstatcache();
		return false;
	}
	if (clear_directory($path)) {
		if (rmdir($path)) {
			clearstatcache();
			return true;
		} else {
			trigger_error('Unable to delete dir: '.$path, E_USER_WARNING);
			clearstatcache();
			return false;
		}
	}
	clearstatcache();
	return false;
}// end delete_directory()


/**
* Deletes all contents of a directory without system calls 
*
* @param string	$path	the directory whose contents to delete
*
* @return bool
* @access public
*/
function clear_directory($path) 
{
	$path  = ereg_replace('/$','',$path);
	if (!is_dir($path)) {
		trigger_error($path.' is not a directory (for clearing).');
		clearstatcache();
		return false;
	}
	$dir = opendir($path);
	while(false !== ($filename = readdir($dir))) {
		if ($filename == '.' || $filename == '..') continue;
		$filename = $path . '/' . $filename;
		if (is_dir($filename)) {
			delete_directory($filename);
		} else {
			if (!unlink($filename)) {
				trigger_error('Unable to delete: '.$filename, E_USER_WARNING);
				clearstatcache();
				return false;
			}
		}
	}
	closedir($dir);
	clearstatcache();
	return true;
}// end clear_directory()


/**
* Recursively copies the contents of a directory without system calls
*
* @param string	$path		the directory to copy
* @param string	$new_path	the parh name to copy to
*
* @return bool
* @access public
*/
function copy_directory($path, $new_path) 
{
	$path     = ereg_replace('/$','',$path);
	$new_path = ereg_replace('/$','',$new_path);
	if (!is_dir($path)) {
		trigger_error($path.' is not a directory (for copying.)');
		clearstatcache();
		return false;
	}
	if(file_exists($new_path)) {
		if(!is_dir($new_path)) {
			trigger_error($new_path.' exists but is not a directory (for copying into).');
			clearstatcache();
			return false;
		}
	} else {
		if(!create_directory($new_path)) {
			clearstatcache();
			return false;
		}
	}
	// Copy the contents
	$dir = opendir($path);
	while(false !== ($filename = readdir($dir))) {
		if ($filename == '.' || $filename == '..') continue;
		$full_filename = $path . '/' . $filename;
		if (is_dir($full_filename)) {
			if(!copy_directory($full_filename, $new_path.'/'.$filename)) {
				clearstatcache();
				return false;
			}
		} else {
			if (!copy($full_filename, $new_path.'/'.$filename)) {
				trigger_error('Unable to copy: '.$full_filename.' --> '.$new_path.'/'.$filename);
				clearstatcache();
				return false;
			}
		}
	}
	closedir($dir);
	clearstatcache();
	return true;
}// end copy_directory()



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
	$to_path = substr($to, 0, strrpos($to, '/'));
	if (!create_directory($to_path)) return false;

	if (!copy ($from, $to)) return false;
	return true;
}// end create_directory()


?>
