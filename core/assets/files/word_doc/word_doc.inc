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
* $Id: word_doc.inc,v 1.24 2012/08/30 01:09:07 ewang Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/files/file/file.inc';
require_once SQ_DATA_PATH.'/private/conf/tools.inc';


/**
* Image
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.24 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Word_Doc extends File
{


	/**
	* All allowed extensions for a word doc
	*
	* Document and templates recognised, for both earlier versions of Word, and
	* Office 2007+ OOXML format (with macros both disabled and enabled).
	*
	* @var Array(string)
	*/
	public $allowed_extensions = Array('doc', 'dot', 'docx', 'docm', 'dotx', 'dotm');


	/**
	* Constructor
	*
	* @param int    $assetid    the asset id to be loaded
	*
	*/
	function __construct($assetid=0)
	{
		parent::__construct($assetid);

	}//end constructor


	/**
	* Returns the raw content of this word document
	*
	* @param boolean $empty_content If TRUE, empty content will be returned
	*
	* @return string
	* @access public
	*/
	function getContent($empty_content=TRUE)
	{
	    $file_info = $this->getExistingFile();
		if (empty($file_info)) return '';
		$file = $file_info['path'];
		
		$content = '';
		
		
		$file_ext = strtolower(substr(strrchr($file, '.'), 1));
        $file = str_replace('"', '\\\"', $file);

        if (SQ_TOOL_TIKA_ENABLED) {
        	if (is_file(SQ_TOOL_JAVA_PATH) && is_executable(SQ_TOOL_JAVA_PATH) && (is_file(SQ_TOOL_TIKA_PATH))) {
				$cache_file = "$file.txtcache";
				// check html cache file
				/*if (file_exists($cache_file) && filemtime($cache_file) >= $this->updated) {
				    $content = file_get_contents($cache_file);
				    return $content;
				}*/

        		// Run the Tika jar file. The -t switch means get text format.
        		$pwd_arg = '';
        		/*$pwd = $this->attr('password');
			    if (empty($pwd) === FALSE) {
        			$pwd_arg = escapeshellarg('--password='.$pwd);
        		}*/

        		$cmd = escapeshellcmd(SQ_TOOL_JAVA_PATH).' -jar '.escapeshellarg(SQ_TOOL_TIKA_PATH).' -t '.$pwd_arg.' --encoding=UTF-8 '.escapeshellarg('file:'.$file);
        			
        		// Run this through a pipe so the password doesn't hit the shell
        		$read_pipe_spec = Array(
        							'pipe',
        							'r'
        		);

        		$log_pipe_spec = Array(
        							'pipe',
        							'w'
        		);

        		// Pipes are defined in the way the child will use them. So Java/Tika will
        		// read from stdin (0) and write to stdout (1) and stderr (2).
        		$spec    = Array(
        					0 => $read_pipe_spec,
        					1 => $log_pipe_spec,
        					2 => $log_pipe_spec,
        				   );
        		$status  = FALSE;
        		$cwd     = NULL;
        		$env     = Array();
        		$process = proc_open($cmd, $spec, $pipes, $cwd, $env);

        		if (is_resource($process) === TRUE) {
        			// Don't block standard error pipe.
        			stream_set_blocking($pipes[2], 0);
        			fclose($pipes[0]);

        			// Find the errors ... do NOT throw to output
				    $errors = stream_get_contents($pipes[2]);
				    fclose($pipes[2]);
				    if (!empty($errors)) {
					    $errors = array_unique(explode(PHP_EOL, $errors));
					    log_error('Error occurred while getting contents for pdf File Asset #'.$this->id.' : '.trim(implode(', ', $errors), ', '), E_USER_WARNING);
				    }//end if

				    // Get the content from stdout after getting the content from stderr
				    $content = stream_get_contents($pipes[1]);
				    fclose($pipes[1]);

        			$retval = proc_close($process);
        			if ((int)$retval !== 0) {
        				trigger_localised_error('CORE0324', sprintf(translate('Error occurred while getting contents for Word Doc Asset #%1$s'), $this->id), E_USER_WARNING);
        			}
        			$content = preg_replace('/^\s+/u', '', $content);
        			$content = preg_replace('/\s+$/u', '', $content);
        		}
        	}

			if (empty($content)) return '';

			// strip out the unnecessary whitespace
			$search = Array('/[\s,]+/u');
			$replace = Array(' ');

        	$content = strtolower(trim(preg_replace($search, $replace, $content)));

        } else if (SQ_TOOL_ANTIWORD_ENABLED) {
            // Run a program that converts Word doc to text, set environment variable of bin_path
            // Can't use Antiword on Word 2007+ files...
            if ($file_ext === 'doc' || $file_ext === 'dot') {
                $cmd = 'ANTIWORDHOME='.SQ_TOOL_ANTIWORD_MAPPING_PATH.' '.SQ_TOOL_ANTIWORD_PATH.' '.$file;
                $content = shell_exec($cmd);
        	}

        }//end if (Tika or Antiword enabled)

        if (!$content) return '';

        // strip out the whitespace
        $content = strtolower(trim(preg_replace('/[\s]+/', ' ', $content)));

		return $content;

	}//end getContent()


}//end class
?>
