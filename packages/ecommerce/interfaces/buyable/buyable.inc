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
* $Id: buyable.inc,v 1.5 2012/08/30 00:58:33 ewang Exp $
*
*/

/**
* Buyable
*
* Purpose
*    Provides an interface to declare an asset as available to the ecommerce system
*
* @author  Luke Wright <lwright@squiz.net>
* @version $Revision: 1.5 $
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