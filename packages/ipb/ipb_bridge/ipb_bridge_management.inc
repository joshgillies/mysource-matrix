<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix Module file is Copyright (c) Squiz Pty Ltd    |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: This Module is not available under an open source       |
* | license and consequently distribution of this and any other files  |
* | that comprise this Module is prohibited. You may only use this     |
* | Module if you have the written consent of Squiz.                   |
* +--------------------------------------------------------------------+
*
* $Id: ipb_bridge_management.inc,v 1.4 2006/12/07 03:17:15 bcaldwell Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* IPB_Bridge_Management
*
* @see Asset_Management
*
* @author  Scott Kim <skim@squiz.net>
* @version $Revision: 1.4 $
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
												'default'		=> 'none',
												'parameters'	=> Array(
																	'options'		=> Array(
																						'none'	=> '-- Select One --',
																						'2_0_4'	=> 'IPB (v2.0.x)',
																						'2_1_3'	=> 'IPB (v2.1.x)',
																					   ),
																	'multiple'		=> FALSE,
																	'allow_empty'	=> FALSE,
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