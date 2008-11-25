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
* $Id: ecommerce_cart_edit_fns.inc,v 1.11 2008/11/25 00:48:03 mbrydon Exp $
*
*/


require_once SQ_PACKAGES_PATH.'/cms/listing_engine/listing_engine_edit_fns.inc';

/**
* Ecommerce_Cart_Edit_Fns
*
* Edit Functions for the shopping cart
*
*
* @author  Luke Wright <lwright@squiz.net>
* @version $Revision: 1.11 $
* @package MySource_Matrix_Packages
* @subpackage ecommerce
*/
class Ecommerce_Cart_Edit_Fns extends Listing_Engine_Edit_Fns
{


	/**
	* Constructor
	*
	* @return void
	* @access public
	*/
	function __construct()
	{
		parent::__construct();

	}//end constructor


}//end class

?>