<?php


require_once SQ_INCLUDE_PATH.'/asset_management.inc';


/**
* Bulkmail_Manager_Management
*
* @see Asset_Management
*
* @author  Nathan de Vries <ndvries@squiz.net>
* @author  Rayn Ong <rong@squiz.net>
*
* @version $Revision: 1.4 $
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