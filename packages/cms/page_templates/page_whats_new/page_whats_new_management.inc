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
* $Id: page_whats_new_management.inc,v 1.12 2005/04/29 05:40:32 gsherwood Exp $
* $Name: not supported by cvs2svn $
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Page_Whats_New_Management
*
* @see Asset_Management
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.12 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Page_Whats_New_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Page_Whats_New_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array(
				'date_field' => Array(
						'added'   => '0.1',
						'type'    => 'selection',
						'default' => 'updated',
						'parameters' => Array(
											'options' => Array(
												'created'	=> 'Created Date',
												'updated'	=> 'Last Updated Date',
												'published'	=> 'Last Published Date',
												),
											),
											'multiple' => false,
											'allow_empty' => false,
										),
				'date_range_from' => Array(
						'added'		=> '0.1',
						'type'		=> 'datetime',
						'default'	=> '---------- --:--:--',
						'parameters' 	=> Array(
							'allow_null' => true,
						),
					),
				'date_range_to' => Array(
						'added'		=> '0.1',
						'type'		=> 'datetime',
						'default'	=> '---------- --:--:--',
						'parameters' 	=> Array(
							'allow_null' => true,
						),
					),
				'date_ranges' => Array(
					'added'		=> '0.1',
					'type'		=> 'serialise',
					'default'	=> Array(),
					),

		);


	}//end constructor

	/**
	* Return an array with information about the events that the what new asset is listening for
	*
	* @access public
	* @return boolean
	*/
	function getEventList()
	{
		return Array(
				Array(
					'event_name'                 => 'requestKeywords',
					'broadcast_type_code'        => 'content_type',
					'broadcast_strict_type_code' => false,
					'options'	=> Array(
						'side_of_link' => 'major',
						'indirect'     => true,
						'is_exclusive' => null,
						'is_dependant' => 1,
						'value'        => '',
						'link_type'    => SQ_LINK_TYPE_2 | SQ_LINK_TYPE_1,
					),
				),
		);

	}//end getEventList()


}//end class

?>
