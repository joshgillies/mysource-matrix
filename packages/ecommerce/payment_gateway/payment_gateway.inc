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
* $Id: payment_gateway.inc,v 1.2 2004/10/26 01:45:15 arailean Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_INCLUDE_PATH.'/asset.inc';

/**
* Payment_Gateway
*
* Abstract asset for other payment gateways to extend
*
*
* @author  Andrei Railean <arailean@squiz.net>
* @version $Revision: 1.2 $
* @package MySource_Matrix_Packages
* @subpackage ecommerce
*/
class Payment_Gateway extends Asset
{

	/**
	* Returns name of the asset
	*
	* @param boolean	$short_name	whether or not we are after the short name or the full name
	*
	* @see Asset::_getName()
	* @access private
	*/
	function _getName($short_name=false)
	{
		return (($short_name) ? $this->attr('short_name') : $this->attr('name'));

	}//end _getName()


}//end class

?>