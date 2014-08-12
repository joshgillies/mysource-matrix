<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd	   |
* | ACN 084 670 600													   |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.														   |
* +--------------------------------------------------------------------+
*
* $Id: ipb_bridge_management.inc,v 1.10 2012/08/30 00:58:55 ewang Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* IPB_Bridge_Management
*
* @see Asset_Management
*
* @author  Scott Kim <skim@squiz.net>
* @version $Revision: 1.10 $
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
												'added'			=> '0.1',
												'type'			=> 'text',
												'default'		=> '',
												'is_admin'		=> FALSE,
												'is_contextable'=> TRUE,
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
																						'2_2_1' => 'IP.Board (v2.2.x)',
																						'3_0_2' => 'IP.Board (v3.0.2)',
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
