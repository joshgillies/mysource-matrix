<?php

/**
* +--------------------------------------------------------------------+
* | Squiz.net Open Source Licence                                      |
* +--------------------------------------------------------------------+
* | Copyright (c), 2003 Squiz Pty Ltd (ABN 77 084 670 600).            |
* +--------------------------------------------------------------------+
* | This source file may be used subject to, and only in accordance    |
* | with, the Squiz Open Source Licence Agreement found at             |
* | http://www.squiz.net/licence.                                      |
* | Make sure you have read and accept the terms of that licence,      |
* | including its limitations of liability and disclaimers, before     |
* | using this software in any way. Your use of this software is       |
* | deemed to constitute agreement to be bound by that licence. If you |
* | modify, adapt or enhance this software, you agree to assign your   |
* | intellectual property rights in the modification, adaptation and   |
* | enhancement to Squiz Pty Ltd for use and distribution under that   |
* | licence.                                                           |
* +--------------------------------------------------------------------+
*
* $Id: search_manager_management.inc,v 1.6 2004/09/02 06:38:48 arailean Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Search_Manager_Management
*
* @see Asset_Management
*
* @author  Dominic Wong <dwong@squiz.net>
* @version $Revision: 1.6 $ - 0.1
* @package MySource_Matrix_Packages
* @subpackage __core__
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
				'description'	=> 'Turns indexing on or off'
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
