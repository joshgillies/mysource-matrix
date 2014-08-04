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
* $Id: workflow_stream_management.inc,v 1.2 2012/08/30 01:09:17 ewang Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Workflow_Stream_Management
*
* @see Asset_Management
*
* @author  Luke Wright <lwright@squiz.com.au>
* @version $Revision: 1.2 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Workflow_Stream_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function __construct(Package_Manager $pm)
	{
		Asset_Management::__construct($pm);

		$this->vars = Array(
						'name'							=> Array(
															'type'			=> 'text',
															'added'			=> '0.1',
															'default'		=> '',
															'description'	=> 'Name of the workflow stream',
														   ),
						'stream_selection_rules'			=> Array(
															'added'			=> '0.1',
															'type'			=> 'serialise',
															'default'		=> Array(),
															'description'	=> 'Rules to determine if this stream should be made available for selection',
														   ),
						'rules_logic'		=>Array(
															'added'			=> '0.2',
															'type'			=> 'selection',
															'default'		=> 'all',
															'parameters'	=> Array(
																				'multiple'		=> FALSE,
																				'allow_empty'	=> FALSE,
																				'options'		=> Array(
																									'all'	=> 'All Rules Must Match',
																									'any'	=> 'Any Rule Matches',
																								   ),
																			   ),
														   ),
					  );

	}//end constructor


	/**
	* Installs this asset
	*
	* In addition to the usual registration of the asset, this should handle
	* the creation of a default workflow stream for existing Workflow Schema
	* assets. Currently existing steps should be moved underneath this default
	* stream.
	*
	* @see Asset_Management::_install()
	* @return boolean
	* @access public
	*/
	function _install()
	{
		try {
			if (Asset_Management::_install() === FALSE) {
				pre_echo('FAIL?');
				return FALSE;
			}
		} catch (Exception $e) {
			pre_echo($e->getMessage());
			return FALSE;
		}

		$ok = TRUE;
		$failed_schema_name = NULL;
		$failed_schemaid = NULL;

		// Create a new "default" workflow stream underneath each
		// Workflow Schema asset
		if ($GLOBALS['SQ_SYSTEM']->am->installed('workflow_schema') === TRUE) {
			$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
			$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

			$schemaids = $GLOBALS['SQ_SYSTEM']->am->getTypeAssetids('workflow_schema');

			if (count($schemaids) > 0) {
				pre_echo('Upgrading '.count($schemaids).' Workflow Schema asset(s) to use Workflow Stream...');

				foreach ($schemaids as $schemaid) {
					$schema = $GLOBALS['SQ_SYSTEM']->am->getAsset($schemaid);
					$new_stream = new Workflow_Stream();
					$new_stream->setAttrValue('name', 'Default Stream');
					echo str_pad('Creating new workflow stream for schema #'.$schemaid, 60);
					$link_info = Array(
									'asset'			=> $schema,
									'link_type'		=> SQ_LINK_TYPE_2,
									'value'			=> 'default_stream',
									'is_exclusive'	=> TRUE,
									'is_dependant'	=> FALSE,
								 );

					if (!$new_stream->create($link_info)) {
						echo '[ FAIL ]'."\n";
						$ok = FALSE;
						$failed_schema_name = $schema->name;
						$failed_schemaid = $schema->id;
						break;
					} else {
						echo '[  OK  ] Id: #'.$new_stream->id."\n";
					}

					// move the workflow steps underneath
					$step_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($schemaid, SQ_LINK_TYPE_2, 'workflow_step');
					foreach ($step_links as $step_link) {
						echo str_pad('Re-linking step #'.$step_link['minorid'].' to stream', 60);
						if ($GLOBALS['SQ_SYSTEM']->am->moveLink($step_link['linkid'], $new_stream->id, SQ_LINK_TYPE_2, $step_link['sort_order']) === 0) {
							echo '[ FAIL ]'."\n";
							$ok = FALSE;
							$failed_schema_name = $schema->name;
							$failed_schemaid = $schema->id;
							break;
						} else {
							echo '[  OK  ]'."\n";
						}
					}

					$GLOBALS['SQ_SYSTEM']->am->forgetAsset($schema);
					unset($schema);
				}

			}//end if there are workflow schemas abounding

			if ($ok === TRUE) {
				pre_echo('Finished upgrading '.count($schemaids).' Workflow Schema asset(s)');
				$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
			} else {
				pre_echo('FAILED upgrading '.count($schemaids).' Workflow Schema asset(s) - failure at schema "'.$failed_schemaid.'" (Id: #'.$failed_schemaid.')');
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			}
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
		}

		return $ok;

	}//end _install()


	/**
	* Returns all files this asset wants installed/updated into the data directory
	*
	* @return array
	* @access private
	*/
	function _getFileList()
	{
		return Array(
				'css/workflow_stream_edit_fns.css',
				'js/workflow_stream_edit_fns.js',
				'files/wf_drag_handle.png',
			   );

	}//end _getFiles()


}//end class

?>
