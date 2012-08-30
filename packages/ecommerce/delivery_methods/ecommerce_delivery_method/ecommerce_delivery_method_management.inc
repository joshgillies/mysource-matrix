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
* $Id: ecommerce_delivery_method_management.inc,v 1.7 2012/08/30 00:58:33 ewang Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Ecommerce_Delivery_Method_Management
*
* @see Asset_Management
*
* @author  Andrei Railean <arailean@squiz.net>
* @version $Revision: 1.7 $
* @package MySource_Matrix_Packages
* @subpackage ecommerce
*/
class Ecommerce_Delivery_Method_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Ecommerce_Delivery_Method_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array(
				'name'	=> Array(
							'added'			=> '0.1',
							'type'			=> 'text',
							'default'		=> '',
							'is_admin'		=> FALSE,
							'is_contextable'=> TRUE,
						   ),
			  );

	}//end constructor


}//end class

?>
