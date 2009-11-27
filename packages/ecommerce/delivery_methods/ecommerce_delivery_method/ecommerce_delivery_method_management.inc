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
* $Id: ecommerce_delivery_method_management.inc,v 1.6 2009/11/27 05:46:06 bpearson Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Ecommerce_Delivery_Method_Management
*
* @see Asset_Management
*
* @author  Andrei Railean <arailean@squiz.net>
* @version $Revision: 1.6 $
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
