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
* $Id: pdf_file.inc,v 1.40 2013/08/20 05:54:12 ewang Exp $
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
* @version $Revision: 1.40 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class PDF_File extends File
{


	/**
	* All allowed extensions for an image
	* @var Array(string)
	*/
	public $allowed_extensions = Array('pdf');


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
	* Returns the raw content of this pdf file
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

		if (SQ_TOOL_PDFTOHTML_ENABLED) {
			$html_file = "$file.html";
			// check html cache file
			if (file_exists($html_file) && filemtime($html_file) >= $this->updated) {
			    $content = file_get_contents($html_file);
			} else {
			    $pwd = $this->attr('password');
			    $pwd = (empty($pwd)) ? '' : $pwd = ' -opw '.$pwd.' ';

			    // Run a program that converts a PDF to HTML, set environment variable of bin_path
			    $cmd = SQ_TOOL_PDFTOHTML_PATH." -i -nomerge -noframes -stdout $pwd $file";
			    $cmd = escapeshellcmd($cmd);
			    // Run the command
			    $log = Array(
						    'pipe',
						    'w',
				       );

			    $status = FALSE;
			    $spec = Array(
						    0	=> Array(
									    'pipe',
									    'r',
							       ),					// stdin is a pipe that the child will read from
						    1	=> $log,				// stdout is a pipe that the child will write to
						    2	=> $log,				// stderr is a file to write to
					    );

			    $cwd = NULL;
			    $env = Array();
			    $process = proc_open($cmd, $spec, $pipes, $cwd, $env);

			    if (is_resource($process)) {

				    // If STDERR is empty, trying to get content from it will hang the process,
				    // as it will wait forever for the data to be written on it, so set it as non-blocking
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

				    // Finally, get the status of this command
				    $status = proc_close($process);

				    // check to see if the command was executed correctly return value is 0,
				    // all good or else log a warning depending upon the error code returned
				    if ($status != '0') {
					    switch ($status) {
						    case 1:
							    $error_str = 'Invalid Argument.';
						    break;

						    case 2:
							    $error_str = 'Invalid Encoded Output.';
						    break;

						    case 3:
							    $error_str = 'Copying text from this Document is not allowed.';
						    break;

						    case 4:
							    $error_str = 'Error Opening File (e.g. corrupted file).';
						    break;

						    default :
							    $error_str = 'Unknown Error.';

					    }//end switch

					    trigger_localised_error('CORE0324', sprintf(translate('Error occurred while getting contents for pdf File Asset #%1$s : %2$s'), $this->id, $error_str), E_USER_WARNING);
					    return '';
				    }//end if
			    }//end if
			}//end else

			if(!empty($content))
			    string_to_file($content, $html_file);
		}//end if


		if (empty($content)) return '';

		// strip out the tags
		$content = trim(strip_tags($content));
		if (empty($content)) return '';


		// strip out the unnecessary whitespace and html entities like &amp;
		$search = Array('/[\s,]+/', '/\&[^;\s]+;/');
		$replace = Array(' ', '');

		$content = strtolower(trim(preg_replace($search, $replace, $content)));



		return $content;

	}//end getContent()


}//end class
?>
