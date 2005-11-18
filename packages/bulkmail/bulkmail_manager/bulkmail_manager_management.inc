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
* @version $Revision: 1.2 $
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
												'added'		=> '0.1',
												'type'		=> 'boolean',
												'default'	=> false,
											   ),
					  );

	}//end constructor


}//end class
?>