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
* $Id: paypal_ipn_receiver_management.inc,v 1.3 2012/08/30 00:58:34 ewang Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Paypal_IPN_Receiver_Management
*
* @see Asset_Management
*
* @author  Anh Ta <ata@squiz.co.uk>
* @version $Revision: 1.3 $
* @package MySource_Matrix_Packages
* @subpackage ecommerce
*/
class Paypal_IPN_Receiver_Management extends Asset_Management
{

	/**
	* Constructor
	*
	*/
	function __construct(&$pm)
	{
		$this->Asset_Management($pm);
		$this->vars = Array(
								'assetid_param_name'	=> Array(
																	'added'		=> '0.1',
																	'type'		=> 'text',
																	'default'	=> '',
														   		),
						   );

	}//end constructor


}//end class
?>