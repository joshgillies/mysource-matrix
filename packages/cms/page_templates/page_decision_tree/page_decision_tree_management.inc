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
* $Id: page_decision_tree_management.inc,v 1.4 2012/08/30 00:58:08 ewang Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Page_Decision_Tree_Management
*
* @see Asset_Management
*
* @author  Mohamed Haidar <mhaidar@squiz.net>
* @version $Revision: 1.4 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Page_Decision_Tree_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function __construct(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array(

						'bodycopy_points'		=> Array(
													'added'		=> '0.0.1',
													'type'		=> 'serialise',
													'default'	=> Array()
												   ),
	
					  );

	}//end constructor
	
	
	/**
	* Return an array with information about the events that this page is listening for
	*
	* @return boolean
	* @access public
	*/
	function getEventList()
	{
		return Array(
				Array(
					'event_name'					=> 'requestKeywords',
					'broadcast_type_code'			=> 'content_type',
					'broadcast_strict_type_code'	=> FALSE,
					'options'						=> Array(
														'side_of_link'	=> 'major',
														'indirect'		=> TRUE,
														'is_exclusive'	=> NULL,
														'is_dependant'	=> TRUE,
														'value'			=> '',
														'link_type'		=> SQ_LINK_TYPE_2,
													   ),
				),
			   );

	}//end getEventList()


	/**
	* Upgrade
	*
	* @param string	$current_version	the version of the asset that is currenly installed
	*
	* @return boolean
	* @access public
	*/
	function _upgrade($current_version)
	{
		if (!parent::_upgrade($current_version)) return FALSE;

		if (version_compare($current_version, '0.2', '<')) {
			pre_echo('UPGRADING DECISION TREE FROM v < 0.2');
			
			$page_ids = $GLOBALS['SQ_SYSTEM']->am->getTypeAssetids('page_decision_tree', TRUE, TRUE);
			foreach ($page_ids as $page_id => $type) {
				$page = $GLOBALS['SQ_SYSTEM']->am->getAsset($page_id, $type['type_code']);
				
				$first_link = $GLOBALS['SQ_SYSTEM']->am->getLink($page->id, SQ_LINK_TYPE_2, 'decision_tree_question', FALSE, 'first_question', 'major', TRUE);
				if (!empty($first_link)){
					$first_question = $GLOBALS['SQ_SYSTEM']->am->getAsset($first_link['minorid']);
					if ($GLOBALS['SQ_SYSTEM']->am->createAssetLink($page, $first_question, SQ_LINK_NOTICE, 'first_question')){
						$GLOBALS['SQ_SYSTEM']->am->updateLink($first_link['linkid'], NULL, '');
					}
				}
				
				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($page);
				unset($page);
			}
		}//end if
		
		return TRUE;

	}//end _upgrade()
	

}//end class

?>
