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
* $Id: design_area_body.inc,v 1.10 2004/07/06 23:20:01 lwright Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_CORE_PACKAGE_PATH.'/designs/design_area/design_area.inc';

/**
* Design_Area_Body
*
* Purpose
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Design_Area_Body extends Design_Area
{

	/*
	* Outputs either the straight HTML or PHP code that is needed for this design area to be represented
	*
	* @param object Design	$design the design for which we are currently printing
	*
	* @return boolean
	* @access public
	*/
	function printArea(&$design)
	{
		$format = $this->attr('format');
		echo '<','?php if (SQ_IN_LIMBO) {$ASSET->printLimbo();} else {';
		switch($format) {
			case 'text':
				// text - text only design - strip images, embeds and objects
				echo 'ob_start();',
					 '$ASSET->printBody();',
				     '$body = ob_get_contents();',
					 'ob_end_clean();',
					 '$body = preg_replace(\'|\<img(.*)\>|\',\'\',$body);',
					 '$body = preg_replace(\'|\\<embed(.*)\>(.*)</embed>|\',\'\',$body);',
					 '$body = preg_replace(\'|\\<object(.*)\>(.*)</object>|\',\'\',$body);',
				     'echo $body;';
				break;

			case 'html':
			default:
				// default = html - just print the body as usual
				echo '$ASSET->printBody();';
				break;

		}
		echo '} ?','>';
		return true;

	}//end printArea()


}//end class

?>
