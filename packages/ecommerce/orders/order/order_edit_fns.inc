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
* $Id: order_edit_fns.inc,v 1.6 2009/07/13 11:14:55 ata Exp $
*
*/


require_once SQ_PACKAGES_PATH.'/cms/form/form_submission/form_submission_edit_fns.inc';

/**
* Order_Edit_Fns
*
* Purpose
*
*
* @author  Luke Wright <lwright@squiz.net>
* @version $Revision: 1.6 $
* @package MySource_Matrix_Packages
* @subpackage ecommerce
*/
class Order_Edit_Fns extends Form_Submission_Edit_Fns
{


	/**
	* Constructor
	*
	* @return void
	* @access public
	*/
	function Order_Edit_Fns()
	{
		$this->Asset_Edit_Fns();

	}//end Order_Edit_Fns()

	
	/**
	* Paints the transaction ID generated during payment processing
	*
	* @param object	Asset				&$rootid	asset being painted
	* @param object Backend_Outputter	&$o			backend outputter
	* @param string						$prefix		prefix for the html doc element name
	*
	* @return boolean
	* @access public
	*/
	function paintTransactionID(&$asset, &$o, $prefix)
	{
		echo '<pre>'.$asset->attr('transaction_id').'</pre>';

		return FALSE;

	}//end paintTransactionID()
	

}//end class

?>