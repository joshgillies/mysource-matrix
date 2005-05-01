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
* $Id: search_manager_management.inc,v 1.8.2.1 2005/05/01 23:33:07 gsherwood Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Search_Manager_Management
*
* @see Asset_Management
*
* @author  Dominic Wong <dwong@squiz.net>
* @version $Revision: 1.8.2.1 $ - 0.1
* @package MySource_Matrix_Packages
* @subpackage search
*/
class Search_Manager_Management extends Asset_Management
{
	/**
	* Constructor
	*/
	function Search_Manager_Management(&$pm)
	{
		$this->Asset_Management($pm);
		$this->vars	= Array(
			'indexing' => Array (
				'added'			=> '0.1',
				'type'			=> 'boolean',
				'default'		=> false,
				'description'	=> 'Turns indexing on or off',
				'parameters' => Array(
									'allow_empty' => false
									),
			),

			'min_word_length' => Array(
				'added'     => '0.1',
				'type'      => 'int',
				'default'   => '4',
				'parameters'=> Array(
					'allow_negative' => false
					),
				),

			'noise_word_list' => Array(
				'added'     => '0.1',
				'type'      => 'serialise',
				'default'   => Array('a','the','it','this'),
			),

			'white_word_list' => Array(
				'added'     => '0.1',
				'type'      => 'serialise',
				'default'   => Array(),
			),

		);

	}//end Search_Manager_Management()


	/**
	* returns an array with information about the events that the form is listening for
	*
	* @access public
	* @return boolean
	*/
	function getEventList()
	{
		return Array(
				Array(
					'event_name'				 => 'assetDeleted',
					'broadcast_type_code'		 => 'global',
					'broadcast_strict_type_code' => true,
					'options'	=> Array(
						'global' => true,
					),
				),
				Array(
					'event_name'				 => 'requestReindex',
					'broadcast_type_code'		 => 'global',
					'broadcast_strict_type_code' => true,
					'options'	=> Array(
						'global' => true,
					),
				),
				Array(
					'event_name'				 => 'indexMetadata',
					'broadcast_type_code'		 => 'global',
					'broadcast_strict_type_code' => true,
					'options'	=> Array(
						'global' => true,
					),
				),
		);
	}//end getEventList()


}//end class
?>
