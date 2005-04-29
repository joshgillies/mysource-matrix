<?php
/**
* +--------------------------------------------------------------------+
* | Squiz.net Commercial Module Licence                                |
* +--------------------------------------------------------------------+
* | Copyright (c) Squiz Pty Ltd (ACN 084 670 600).                     |
* +--------------------------------------------------------------------+
* | This source file is not open source or freely usable and may be    |
* | used subject to, and only in accordance with, the Squiz Commercial |
* | Module Licence.                                                    |
* | Please refer to http://www.squiz.net/licence for more information. |
* +--------------------------------------------------------------------+
*
* $Id: form_question_type_file_upload_edit_fns.inc,v 1.5 2005/04/29 05:40:30 gsherwood Exp $
*
*/


require_once dirname(__FILE__).'/../../form_question/form_question_edit_fns.inc';

/**
* Form_Question_Type_File_Upload_Edit_Fns
*
* Purpose
*
*
* @author  Luke Wright <lwright@squiz.net>
* @version $Revision: 1.5 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Form_Question_Type_File_Upload_Edit_Fns extends Form_Question_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function Form_Question_Type_File_Upload_Edit_Fns()
	{
		$this->Form_Question_Edit_Fns();

	}//end constructor


	function paintFileSizeNote(&$asset, &$o, $prefix)
	{
		$upload_size = strtolower(ini_get('upload_max_filesize'));

		// allow users to specify abbreviated forms of file size (kB, MB, GB)
		// the conversion used is 1024 to the kByte, rather than the decimal 1000
		if (substr($upload_size,-1) == 'k') {
			$upload_size = floatval($upload_size) * 1024;
		} else if (substr($upload_size,-1) == 'm') {
			$upload_size = floatval($upload_size) * 1024 * 1024;
		}

		$post_size = strtolower(ini_get('post_max_size'));

		// allow users to specify abbreviated forms of file size (kB, MB, GB)
		// the conversion used is 1024 to the kByte, rather than the decimal 1000
		if (substr($post_size,-1) == 'k') {
			$post_size = floatval($post_size) * 1024;
		} else if (substr($post_size,-1) == 'm') {
			$post_size = floatval($post_size) * 1024 * 1024;
		}

		$mem_limit_size = strtolower(ini_get('memory_limit'));

		// allow users to specify abbreviated forms of file size (kB, MB, GB)
		// the conversion used is 1024 to the kByte, rather than the decimal 1000
		if (substr($mem_limit_size,-1) == 'k') {
			$mem_limit_size = floatval($mem_limit_size) * 1024;
		} else if (substr($mem_limit_size,-1) == 'm') {
			$mem_limit_size = floatval($mem_limit_size) * 1024 * 1024;
		}

		?><i>Please note that your PHP configuration specifies a maximum file upload size of <b><?php echo strtoupper(easy_filesize(min($upload_size, $post_size, $mem_limit_size))) ?></b>.<br/>
If you enter a larger value than this then be aware that the configuration setting will override it.</i><br/><br/>
Comparison values for the file size rule this can be entered in shorthand notation: 'K' or 'KB' for kilobytes, 'M' or 'MB' for megabytes.<?php

	}//end paintFileSizeNote()


	function processFileSizeNote(&$asset, &$o, $prefix)
	{
		return true;

	}//end processFileSizeNote()


}//end class
?>
