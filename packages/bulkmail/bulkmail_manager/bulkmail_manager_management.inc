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
* $Id: bulkmail_manager_management.inc,v 1.9 2012/08/30 00:57:13 ewang Exp $
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
* @version $Revision: 1.9 $
* @package MySource_Matrix_Packages
* @subpackage bulkmail
*/
class Bulkmail_Manager_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function __construct(&$pm)
	{
		parent::__construct($pm);

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
