<?php
/**
* +- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
* | This MySource Matrix Module file is Copyright (c) Squiz Pty Ltd    |
* | ACN 084 670 600													   |
* +- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
* | IMPORTANT: This Module is not available under an open source	   |
* | license and consequently distribution of this and any other files  |
* | that comprise this Module is prohibited. You may only use this	   |
* | Module if you have the written consent of Squiz.				   |
* +- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
*
* $Id: soap_server_error_handler.inc,v 1.3.2.1 2009/04/29 05:02:45 hnguyen Exp $
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
