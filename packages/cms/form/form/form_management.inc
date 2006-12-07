<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: form_management.inc,v 1.44 2006/12/07 00:04:17 emcdonald Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Form_Management
*
* @see Asset_Management
*
* @author  Marc McIntyre <mmcintyre@squiz.net>
* @version $Revision: 1.44 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Form_Management extends Asset_Management
{


	/**
	* Constructor
	*
	* @return void
	* @access public
	*/
	function Form_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array(
						'name'						=> Array(
														'added'		=> '0.0.1',
														'type'		=> 'text',
														'default'	=> '',
														'update_fn'	=> '',
													   ),
						'log_submissions'			=> Array(
														'added'			=> '0.0.1',
														'type'			=> 'boolean',
														'default'		=> TRUE,
														'parameters'	=> Array(
																			'allow_empty'	=> FALSE,
																		   ),
													   ),
						'show_results'				=> Array(
														'added'			=> '0.0.1',
														'type'			=> 'boolean',
														'default'		=> TRUE,
														'parameters'	=> Array(
																			'allow_empty'	=> FALSE,
																		   ),
													   ),
						'submit_button'				=> Array(
														'added'		=> '0.0.1',
														'type'		=> 'text',
														'default'	=> '',
													   ),
						'reset_button'				=> Array(
														'added'		=> '0.0.1',
														'type'		=> 'text',
														'default'	=> '',
													   ),
						'question_col_width'		=> Array(
														'added'		=> '0.0.1',
														'type'		=> 'html_width',
														'default'	=> '40%',
													   ),
						'answer_col_width'			=> Array(
														'added'		=> '0.0.1',
														'type'		=> 'html_width',
														'default'	=> '60%',
													   ),
						'form_width'				=> Array(
														'added'		=> '0.0.1',
														'type'		=> 'html_width',
														'default'	=> '100%',
													   ),
						'sort_order'				=> Array(
														'added'		=> '0.0.1',
														'type'		=> 'serialise',
														'default'	=> Array(),
													   ),
						'questions'					=> Array(
														'added'		=> '0.0.1',
														'type'		=> 'serialise',
														'default'	=> Array(),
													   ),
						'use_bodycopy_main'			=> Array(
														'added'			=> '0.0.1',
														'type'			=> 'boolean',
														'default'		=> FALSE,
														'parameters'	=> Array(
																			'allow_empty'	=> FALSE,
																		   ),
													   ),
						'use_bodycopy_thank_you'	=> Array(
														'added'			=> '0.0.1',
														'type'			=> 'boolean',
														'default'		=> FALSE,
														'parameters'	=> Array(
																			'allow_empty'	=> FALSE,
																		   ),
													   ),
						'use_client_side'			=> Array(
														'added'			=> '0.0.1',
														'type'			=> 'boolean',
														'default'		=> FALSE,
														'parameters'	=> Array(
																			'allow_empty'	=> FALSE,
																		   ),
													   ),
						'note_size'					=> Array(
														'added'		=> '0.0.1',
														'type'		=> 'text',
														'default'	=> '80%',
													   ),
						'next_questionid'			=> Array(
														'added'		=> '0.3',
														'type'		=> 'int',
														'default'	=> 0,
													   ),
					  );

	}//end constructor


	/**
	* returns an array with information about the events that the form is listening for
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
														'is_dependant'	=> 1,
														'value'			=> '',
														'link_type'		=> SQ_LINK_TYPE_2 | SQ_LINK_TYPE_1,
													   ),
				),
			   );

	}//end getEventList()


	/**
	* Gets a list of all metadata files that we want web accessable
	*
	* @return array
	* @access private
	*/
	function _getFileList()
	{
		return Array(
				'popups/page_contents_keywords.php',
				'popups/thank_you_keywords.php',
			   );

	}//end _getFileList()


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
		// first do things as normal
		$res = parent::_upgrade($current_version);

		if (version_compare($current_version, '0.6', '<')) {
			pre_echo('UPGRADING FORM TO v0.6');
			$db =& $GLOBALS['SQ_SYSTEM']->db;
			$descendants = $GLOBALS['SQ_SYSTEM']->am->getTypeDescendants('form', TRUE);
			foreach ($descendants as $i => $desc) {
				$descendants[$i] = $db->quote($desc);
			}
			$form_assetids =  $db->getCol('SELECT assetid FROM sq_ast WHERE type_code IN ('.implode(',', $descendants).')');
			assert_valid_db_result($form_assetids);

			foreach ($form_assetids as $form_id) {
				$form =& $GLOBALS['SQ_SYSTEM']->am->getAsset($form_id);
				// delete the content file so it gets regenerated next run
				if (is_file($form->data_path.'/content_file.php')) {
					unlink($form->data_path.'/content_file.php');
				}
				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($form);
			}
			pre_echo('DONE');
		}


		if (version_compare($current_version, '0.7', '<')) {
			pre_echo('UPGRADING FORM TO v0.7');
			$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
			$db =& $GLOBALS['SQ_SYSTEM']->db;

			$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

			// It's going to be best to do this via an SQL query
			$sql = 'UPDATE
						sq_ast_lnk
					SET
						is_dependant = 1
					WHERE
						majorid IN (
									SELECT
										assetid
									FROM
										sq_ast
									WHERE
										type_code IN (
														SELECT
															type_code
														FROM
															sq_ast_typ_inhd
														WHERE
															inhd_type_code = '.$db->quoteSmart('form').'
													 )
								   )
					AND
						minorid IN (
									SELECT
										assetid
									FROM
										sq_ast
									WHERE
										type_code IN (
														SELECT
															type_code
														FROM
															sq_ast_typ_inhd
														WHERE
															inhd_type_code = '.$db->quoteSmart('form_section').'
													 )
								   )';
			$result = $db->query($sql);
			assert_valid_db_result($result);

			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			pre_echo('DONE');
		}//end if

		if (version_compare($current_version, '0.8', '<')) {
			pre_echo('UPGRADING FORM TO v0.8');
			$db =& $GLOBALS['SQ_SYSTEM']->db;
			$descendants = $GLOBALS['SQ_SYSTEM']->am->getTypeDescendants('form', TRUE);
			foreach ($descendants as $i => $desc) {
				$descendants[$i] = $db->quote($desc);
			}
			$form_assetids =  $db->getCol('SELECT assetid FROM sq_ast WHERE type_code IN ('.implode(',', $descendants).')');
			assert_valid_db_result($form_assetids);

			pre_echo('Upgrading '.count($form_assetids).' asset(s)...');

			foreach ($form_assetids as $form_id) {
				$form =& $GLOBALS['SQ_SYSTEM']->am->getAsset($form_id);
				// delete the content file so it gets regenerated next run
				if (is_file($form->data_path.'/content_file.php')) {
					unlink($form->data_path.'/content_file.php');
				}
				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($form);
			}

			pre_echo('DONE');
		}

		return $res;

	}//end _upgrade()


}//end class
?>
