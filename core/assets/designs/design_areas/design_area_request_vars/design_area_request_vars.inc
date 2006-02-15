<?php
/**
* +--------------------------------------------------------------------+
* | Squiz.net Open Source Licence                                      |
* +--------------------------------------------------------------------+
* | Copyright (c), 2003 Squiz Pty Ltd (ABN 77 084 670 600).            |
* +--------------------------------------------------------------------+
* | This source file may be used subject to, and only in accordance    |
* | with, the Squiz Open Source Licence Agreement found at             |
* | http://www.squiz.net/licence.                                      |
* | Make sure you have read and accept the terms of that licence,      |
* | including its limitations of liability and disclaimers, before     |
* | using this software in any way. Your use of this software is       |
* | deemed to constitute agreement to be bound by that licence. If you |
* | modify, adapt or enhance this software, you agree to assign your   |
* | intellectual property rights in the modification, adaptation and   |
* | enhancement to Squiz Pty Ltd for use and distribution under that   |
* | licence.                                                           |
* +--------------------------------------------------------------------+
*
* $Id: design_area_request_vars.inc,v 1.5 2006/02/15 02:55:37 sdanis Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/designs/design_area/design_area.inc';

/**
* Design_Area_Request_Vars
*
* Purpose
*	Display something from the request (GET or POST) in the design
*
* @author  Luke Wright <lwright@squiz.net>
* @version $Revision: 1.5 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Design_Area_Request_Vars extends Design_Area
{


	/**
	* Outputs the var either as straight HTML or PHP code, depending on the type
	*
	* @param string	$var	the name of the var to print out
	* @param array	$args	array of any extra arguments that existed in the print command
	*
	* @return void
	* @access public
	*/
	function printVar($var, $args)
	{
		switch ($this->vars['method']['value']) {
			case 'get':
				$target_vars = '$_GET';
			break;

			case 'post':
				$target_vars = '$_POST';
			break;

			case 'request':
			default:
				$target_vars = '$_REQUEST';
			break;

		}

		echo '<?php echo array_get_index('.$target_vars.', \''.addslashes($var).'\', \'\') ?>';

	}//end printVar()


}//end class
?>