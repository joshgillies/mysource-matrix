<?php
/**
* +--------------------------------------------------------------------+
* | Squiz.net Commercial Module Licence                                |
* +--------------------------------------------------------------------+
* | Copyright (c) Squiz Pty Ltd (ACN 084 670 600).                     |
* +--------------------------------------------------------------------+
* | This source file is not open source or freely usable and may be    |
* | used subject to, and only in accordance with, the Squiz Commercial |
* | Module Licence.                                                    |
* | Please refer to http://www.squiz.net/licence for more information. |
* +--------------------------------------------------------------------+
*
* $Id: ecommerce_delivery_method_management.inc,v 1.2 2005/07/21 04:56:36 lwright Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Ecommerce_Delivery_Method_Management
*
* @see Asset_Management
*
* @author  Andrei Railean <arailean@squiz.net>
* @version $Revision: 1.2 $
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
							'added'		=> '0.1',
							'type'		=> 'text',
							'default'	=> '',
						   ),
			  );

	}//end constructor


}//end class

?>
