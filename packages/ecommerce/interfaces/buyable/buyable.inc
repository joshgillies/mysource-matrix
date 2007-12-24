<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix Module file is Copyright (c) Squiz Pty Ltd    |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: This Module is not available under an open source       |
* | license and consequently distribution of this and any other files  |
* | that comprise this Module is prohibited. You may only use this     |
* | Module if you have the written consent of Squiz.                   |
* +--------------------------------------------------------------------+
*
* $Id: buyable.inc,v 1.3 2007/12/24 02:59:02 colivar Exp $
*
*/

/**
* Buyable
*
* Purpose
*    Provides an interface to declare an asset as available to the ecommerce system
*
* @author  Luke Wright <lwright@squiz.net>
* @version $Revision: 1.3 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
interface Buyable
{


	/**
	* Returns a price of the asset
	*
	* @return float
	* @access public
	*/
	public function getPrice();//end getPrice()


}//end class

?>