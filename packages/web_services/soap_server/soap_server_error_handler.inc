<?php
/**
* +- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ACN 084 670 600													   |
* +- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.                                                        |
* +- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
*
* $Id: soap_server_error_handler.inc,v 1.6 2012/08/30 00:59:46 ewang Exp $
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
	function soap_server_error_handler($err_no, $err_msg, $err_file, $err_line)
	{
		if ($err_no & (E_USER_ERROR | E_ERROR | E_USER_WARNING)) {
			log_dump("SOAP SERVER ERROR: ".$err_msg .'. Error Number:'.$err_no .'. Error File:'.$err_file.'. Error Line:'.$err_line);
			throw new SoapFault("SERVER", $err_msg);
		}//end if

	}//end soap_server_error_handler()


?>
