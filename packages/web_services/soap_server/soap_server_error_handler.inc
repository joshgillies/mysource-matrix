<?php
/**
* +- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ACN 084 670 600                                                    |
* +- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.                                                        |
* +- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
*
* $Id: soap_server_error_handler.inc,v 1.1 2008/05/09 04:54:02 hnguyen Exp $
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
		throw new SoapFault("SERVER", $err_msg);

	}//end soap_server_error_handler()


?>