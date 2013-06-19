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
* $Id: js_api_error_handler.inc,v 1.6 2013/06/19 05:10:36 lwright Exp $
*
*/

/**
* This function handle error thrown by Matrix
*
* @param int    $err_no     The type of error (E_*)
* @param string $err_msg    The error message
* @param string $err_file   The file the error occured in
* @param string $err_line   The line the error occured on
*
* @return void
* @access public
*/
function js_api_error_handler($err_no, $err_msg, $err_file, $err_line)
{
	if ((defined('E_RECOVERABLE_ERROR') && $err_no == E_RECOVERABLE_ERROR) ||
		($err_no & (E_USER_ERROR | E_ERROR | E_USER_WARNING))) {
		log_dump(strtoupper(get_error_name($err_no)).': '.$err_msg);
	}//end if

}//end js_api_error_handler()


?>
