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
* $Id: ecommerce_order_management.inc,v 1.1 2005/07/22 07:28:55 skim Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Ecommerce_Order_Management
*
* @see Asset_Management
*
* @author  Scott Kim <skim@squiz.net>
* @version $Revision: 1.1 $
* @package MySource_Matrix_Packages
* @subpackage ecommerce
*/
class Ecommerce_Order_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Ecommerce_Order_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array(
						'summary'			=> Array(
												'added'		=> '0.1',
												'type'		=> 'text',
												'default'	=> '',
											   ),
						'products'			=> Array(
												'added'		=> '0.1',
												'type'		=> 'serialise',
												'defalut'	=> Array(),
											   ),
						'processing_data'	=> Array(
												'added'		=> '0.1',
												'type'		=> 'serialise',
												'defalut'	=> Array(),
											   ),
					  );

	}//end constructor


}//end class

?>
