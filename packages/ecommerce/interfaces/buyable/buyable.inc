<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: buyable.inc,v 1.4 2008/11/25 00:48:04 mbrydon Exp $
*
*/

/**
* Buyable
*
* Purpose
*    Provides an interface to declare an asset as available to the ecommerce system
*
* @author  Luke Wright <lwright@squiz.net>
* @version $Revision: 1.4 $
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