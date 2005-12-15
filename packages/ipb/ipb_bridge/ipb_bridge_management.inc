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
* $Id: ipb_bridge_management.inc,v 1.1.1.1 2005/12/15 03:02:07 skim Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* IPB_Bridge_Management
*
* @see Asset_Management
*
* @author  Scott Kim <skim@squiz.net>
* @version $Revision: 1.1.1.1 $
* @package MySource_Matrix_Packages
* @subpackage ipb
*/
class IPB_Bridge_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function IPB_Bridge_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array(
						'name'				=> Array(
												'added'		=> '0.1',
												'type'		=> 'text',
												'default'	=> '',
											   ),
						'version'			=> Array(
												'added'			=> '0.1',
												'type'			=> 'selection',
												'default'		=> '2_0_4',
												'parameters'	=> Array(
																	'options'		=> Array(
																						'2_0_4'	=> 'IPB (v2.0.4)',
																					   ),
																	'multiple'		=> false,
																	'allow_empty'	=> false,
																   ),
											   ),
						'location'			=> Array(
												'added'		=> '0.1',
												'type'		=> 'text',
												'default'	=> '',
											   ),
						'user_attributes'	=> Array(
												'added'		=> '0.1',
												'type'		=> 'serialise',
												'default'	=> Array(),
											   ),
					  );

	}//end constructor


}//end class

?>