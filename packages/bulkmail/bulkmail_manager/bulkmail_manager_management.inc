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
* $Id: bulkmail_manager_management.inc,v 1.5 2006/12/07 05:55:56 bcaldwell Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';


/**
* Bulkmail_Manager_Management
*
* @see Asset_Management
*
* @author  Nathan de Vries <ndvries@squiz.net>
* @author  Rayn Ong <rong@squiz.net>
*
* @version $Revision: 1.5 $
* @package MySource_Matrix_Packages
* @subpackage bulkmail
*/
class Bulkmail_Manager_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Bulkmail_Manager_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array(
						'use_bm_threshold'	=> Array(
												'added'			=> '0.1',
												'type'			=> 'boolean',
												'default'		=> FALSE,
												'description'	=> 'Whether to override threshold settings',
											   ),
						'chunk_size'		=> Array(
												'added'			=> '0.1',
												'type'			=> 'int',
												'default'		=> 100,
												'parameters'	=> Array(
																	'allow_negative'	=> FALSE,
																	'allow_empty'		=> FALSE,
																	'range_lower'		=> 1,
																	'range_upper'		=> 600,
																   ),
												'description'	=> 'Allow pausing after this number of recipients',
											   ),
					  );

	}//end constructor


}//end class
?>