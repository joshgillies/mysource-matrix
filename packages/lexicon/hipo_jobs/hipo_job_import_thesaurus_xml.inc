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
* $Id: hipo_job_import_thesaurus_xml.inc,v 1.5 2005/04/15 00:37:56 lwright Exp $
*
*/

require_once SQ_SYSTEM_ROOT.'/core/hipo/hipo_job.inc';

/**
* Highly Intensive Processing Object (HIPO) to import thesaurus xml
*
* Initial Running Vars :
*    thesaurus_id	-	the ID of the thesaurus that is being added to
*
* @author  Andrei Railean <arailean@squiz.net>
* @version $Revision: 1.5 $
* @package MySource_Matrix
* @subpackage HIPO
*/
class HIPO_Job_Import_Thesaurus_XML extends HIPO_Job
{

	/**
	* The name of this HIPO job (should not be modified by code)
	* @var string
	*/
	var $hipo_name = 'Import Thesaurus XML';

	/**
	* Array of steps to perform in this HIPO
	* @var Array
	*/
	var $_steps = Array(
		Array(
			'name'             => 'Starting up process',
			'function_call'    => Array(
				'process_function' => 'processStart',
			),
			'running_mode'     => 'server',
			'auto_step'        => true,
			'allow_cancel'	   => false,
			'percent_done'     => 0,
			'complete'         => false,
			'message'          => 'Setting up the environment',
		),

		Array(
			'name'			   => 'Acquiring Permission Locks',
			'hipo_job'		   => Array(
				'init_details_function' => 'getLockAssetDetails',
			),
			'running_mode'	   => 'server',
			'auto_step'		   => true,
			'allow_cancel'	   => true,
			'percent_done'	   => 0,
			'complete'		   => false,
			'message'		   => '',
		),

		Array(
			'name'             => 'Parsing XML file',
			'function_call'    => Array(
				'process_function' => 'processParseXML',
			),
			'running_mode'     => 'server',
			'skip_step'        => false,
			'auto_step'        => true,
			'allow_cancel'	   => false,
			'percent_done'     => 5,
			'complete'         => false,
			'message'          => '',
		),

		Array(
			'name'             => 'Erasing Thesaurus Contents',
			'function_call'    => Array(
				'process_function' => 'processEraseContents',
			),
			'running_mode'     => 'server',
			'auto_step'        => false,
			'allow_cancel'	   => true,
			'percent_done'     => 1,
			'complete'         => false,
			'message'          => 'Please confirm that you want to erase the existing thesaurus. Your XML file looks valid and is ready to be imported, its contents will replace the existing thesaurus data.',
		),

		Array(
			'name'             => 'Adding Relations to the Database',
			'function_call'    => Array(
				'process_function' => 'processAddRelations',
			),
			'running_mode'     => 'server',
			'auto_step'        => true,
			'allow_cancel'	   => false,
			'percent_done'     => 25,
			'complete'         => false,
			'message'          => '',
		),

		Array(
			'name'             => 'Adding Entities to the Database',
			'function_call'    => Array(
				'process_function' => 'processAddEntities',
			),
			'running_mode'     => 'server',
			'auto_step'        => true,
			'allow_cancel'	   => false,
			'percent_done'     => 60,
			'complete'         => false,
			'message'          => '',
		),

		Array(
			'name'             => 'Linking Entities Together',
			'function_call'    => Array(
				'process_function' => 'processLinkEntities',
			),
			'running_mode'     => 'server',
			'auto_step'        => true,
			'allow_cancel'	   => false,
			'percent_done'     => 95,
			'complete'         => false,
			'message'          => '',
		),

		Array(
			'name'             => 'Finalizing Import',
			'function_call'    => Array(
				'process_function' => 'processFinish',
			),
			'running_mode'     => 'server',
			'auto_step'        => true,
			'allow_cancel'	   => false,
			'percent_done'     => 0,
			'complete'         => false,
			'message'          => 'Cleaning up and moving out',
		),

	);


	/**
	* Constructor
	*
	* @param string	$code_name		a unique codename the HIPO
	*/
	function HIPO_Job_Import_Thesaurus_XML($code_name='')
	{
		$this->uses_trans = true;
		$this->HIPO_Job($code_name);

	}//end constructor


	/**
	* Returns a unique codename the HIPO can use to ensure it is not being run twice
	*
	* @return string
	* @access public
	*/
	function getCodeName()
	{
		return parent::getCodeName().'-'.$this->_running_vars['thesaurus_id'];

	}//end getCodeName()


	/**
	* Prepares the running vars and gets the children of the asset in question.
	*
	* @return boolean
	* @access public
	*/
	function prepare()
	{
		// to start regenerating metadata, we need to have a root assetid - default to root folder if none is supplied
		if (is_null($this->_running_vars['thesaurus_id'])) {
			trigger_localised_error('LEX0002', E_USER_WARNING);
			return false;
		}

		// so we have an assetid to start at, but make sure it is a valid assetid
		$asset = &$this->getThesaurusRef();
		if (is_null($asset)) {
			trigger_localised_error('LEX0003', E_USER_WARNING, $this->_running_vars['thesaurus_id']);
			return false;
		}

		return parent::prepare();

	}//end prepare()


	/**
	* Dummy function that runs an empty step, so that there is no empty screen
	* before the end of the first real step
	*
	* @param array $step_data    step data
	* @param string $prefix      unique prefix
	*
	* @access public
	* @return boolean
	*/
	function processStart(&$step_data, $prefix)
	{
		$step_data['percent_done'] = 100;
		$step_data['complete'] = true;
		return true;

	}//end processStart()


	/**
	* Erase Thesaurus Contents
	*
	* @param array $step_data    step data
	* @param string $prefix      unique prefix
	*
	* @access public
	* @return boolean
	*/
	function processEraseContents(&$step_data, $prefix)
	{

		$th_asset = &$this->getThesaurusRef();

		if (!$th_asset->erase()) {
			trigger_localised_error('LEX0004', E_USER_WARNING);
			return false;
		}

		$step_data['percent_done'] = 100;
		$step_data['complete'] = true;

		return true;

	}//end processEraseContents()


	/**
	* Parse the supplied XML file
	*
	* @param array $step_data    step data
	* @param string $prefix      unique prefix
	*
	* @access public
	* @return boolean
	*/
	function processParseXML(&$step_data, $prefix)
	{

		require_once 'XML/Tree.php';

		$th_asset = &$this->getThesaurusRef();
		$import_path = $th_asset->getImportPath();

		$input = new XML_Tree($import_path);
		$root  = &$input->getTreeFromFile();
		if (PEAR::isError($root)) {
			trigger_localised_error('LEX0005', E_USER_WARNING);
			return false;
		}

		if ($root->name != 'thesaurus') {
			trigger_localised_error('LEX0006', E_USER_WARNING);
			return false;
		}

		// process the XML regardless of the order of elements
		$child_nodes = $root->children;
		$total_links = 0;

		$relations = $entities = $entity_relations = Array();

		foreach ($child_nodes as $child) {
			switch ($child->name) {

				// RELATIONS
				// prepare relations for addition to the database
				case 'relations':
					foreach ($child->children as $relation) {
						$rel_name = $relation->attributes['name'];
						$relations[$rel_name] = $rel_name;
					}
				break;

				// ENTITIES
				case 'entities':
					$entity_relations = Array();

					foreach ($child->children as $entity) {
						$ent_name = $entity->attributes['name'];
						$ent_relations = Array();
						$ent_description = null;

						// Entity Has 2 sub-elements: description and relation
						// process them regardless of order
						foreach ($entity->children as $entity_node) {
							switch ($entity_node->name) {

								// if there are multiple 'description' tags
								// use the last one
								case 'description':
									$ent_description = $entity_node->content;
								break;

								// relations are prepared for post-processing
								// this is required because before the specific relation can be recorded
								// the relation type and both the entities have to be already in the system
								case 'relation':
									$rel_name = $entity_node->attributes['name'];
									$rel_entities = Array();
									foreach ($entity_node->children as $related_entity) {
										if ($related_entity->name == 'entity') {
											$er_name = array_get_index($related_entity->attributes,'name');
											if (isset($er_name)) {
												$rel_entities[] = $er_name;
												$total_links++;
											}
										}
									}

									// in order for the relation to be recorded
									// we make sure that the name and the related entities were specified
									// currently we are ignoring invalid records
									if (!empty($rel_entities) && !empty($rel_name)) {
										$ent_relations[$rel_name] = $rel_entities;
									}

								break;
							}
						}

						$entities[$ent_name] = Array(
							'name'        => $ent_name,
							'description' => $ent_description,
						);
						$entity_relations[$ent_name] = $ent_relations;
					}

				break;
			}
		}

		$this->_running_vars['relations']        = $relations;
		$this->_running_vars['entities']         = $entities;
		$this->_running_vars['entity_relations'] = $entity_relations;
		$this->_running_vars['entity_count']     = sizeof($entities);
		$this->_running_vars['er_count']         = sizeof($entity_relations);
		$this->_running_vars['link_count']       = $total_links;
		$this->_running_vars['processed_links']  = 0;
		$this->_running_vars['entity_map']       = Array();

		$step_data['percent_done'] = 100;
		$step_data['complete'] = true;

		return true;

	}//end processParseXML()


	/**
	* Add relations to the thesaurus
	*
	* @param array $step_data    step data
	* @param string $prefix      unique prefix
	*
	* @access public
	* @return boolean
	*/
	function processAddRelations(&$step_data, $prefix)
	{
		$th_asset = &$this->getThesaurusRef();

		$relations = array_get_index($this->_running_vars,'relations');

		foreach ($relations as $relation) {
			$th_asset->addRelation($relation);
		}

		$th_asset->saveAttributes();

		$step_data['percent_done'] = 100;
		$step_data['complete'] = true;

		return true;

	}//end processAddRelations()


	/**
	* Add entities to the thesaurus
	*
	* Processes new entities in batches
	*
	* @param array $step_data    step data
	* @param string $prefix      unique prefix
	*
	* @access public
	* @return boolean
	*/
	function processAddEntities(&$step_data, $prefix)
	{

		$entities_per_step = 500;
		$i = 0;

		$th_asset = &$this->getThesaurusRef();

		$total_count  = $this->_running_vars['entity_count'];

		if ($total_count == 0) {
			trigger_localised_error('LEX0007', E_USER_WARNING);
			$step_data['complete'] = true;
			return true;
		}

		// add entities and create a temporary "entity map" that stores a mapping
		// from entity name to entity ID for speed improvements. this map is relied upon
		// by the function that links entities together
		foreach ($this->_running_vars['entities'] as $name => $entity) {
			$id = $th_asset->addEntity($name, $entity['description']);
			unset($this->_running_vars['entities'][$name]);
			$this->_running_vars['entity_map'][$name] = $id;

			if ($i++ > $entities_per_step) {
				break;
			}
		}

		// determine the status of this step
		$processed_count = sizeof($this->_running_vars['entity_map']);

		$step_data['percent_done'] = ($processed_count/$total_count)*100;
		if (empty($this->_running_vars['entities'])) {
			$step_data['complete'] = true;
		} else {
			$step_data['complete'] = false;
		}

		$step_data['message']  = "Processed $processed_count out of $total_count entities";

		return true;

	}//end processAddEntities()


	/**
	* Link entitites together
	*
	* @param array $step_data    step data
	* @param string $prefix      unique prefix
	*
	* @access public
	* @return boolean
	*/
	function processLinkEntities(&$step_data, $prefix)
	{

		$entities_per_step = 100;
		$i = $links = 0;

		$th_asset = &$this->getThesaurusRef();

		$map = $this->_running_vars['entity_map'];

		$total_count    = $this->_running_vars['link_count'];
		if ($total_count == 0) {
			$step_data['complete'] = true;
			return true;
		}

		$bad_id = $th_asset->BAD_ID;

		foreach ($this->_running_vars['entity_relations'] as $name => $rel_entities) {
			$parent_id = $this->_running_vars['entity_map'][$name];
			foreach ($rel_entities as $rel_name => $child_entities) {
				$rel = $th_asset->getRelationByName($rel_name);

				if (empty($rel)) {
					trigger_localised_error('LEX0008', E_USER_WARNING, $name, $rel_name);
					continue;
				}

				foreach ($child_entities as $child_name) {
					$child_id = array_get_index($this->_running_vars['entity_map'], $child_name, $bad_id);
					if ($child_id != $bad_id) {
						$th_asset->connectEntities($parent_id, $child_id, $rel['id'] );
						$this->_running_vars['processed_links']++;
					}
				}
			}

			unset($this->_running_vars['entity_relations'][$name]);

			if ($i++ > $entities_per_step) {
				break;
			}
		}

		$processed_count = $this->_running_vars['processed_links'];
		$step_data['percent_done'] = ($processed_count/$total_count) * 100;

		if (empty($this->_running_vars['entity_relations'])) {
			$step_data['complete'] = true;
		} else {
			$step_data['complete'] = false;
		}

		$step_data['message']  = "Created $processed_count links out of $total_count.";

		return true;

	}//end processLinkEntities()


	/**
	* Finalize import
	*
	* Relies on the thesaurus to set a few variables to signify that the
	* import was successful
	*
	* @param array $step_data    step data
	* @param string $prefix      unique prefix
	*
	* @access public
	* @return boolean
	*/
	function processFinish(&$step_data, $prefix)
	{
		$th_asset = &$this->getThesaurusRef();

		$th_asset->finishImport();

		$step_data['percent_done'] = 100;
		$step_data['complete'] = true;

		return true;

	}//end processFinish()


	/**
	* This function is used to set-up the details for the locking the assets by the nested HIPO_Job
	*
	* @param string		&$job_type
	* @param Array()	&$running_vars
	* @param Array()	&$options
	*
	* @return void
	* @access public
	*/
	function getLockAssetDetails(&$job_type, &$running_vars, &$options)
	{
		$job_type = 'hipo_job_acquire_locks';

		$options['auto_complete'] = true;

		$running_vars = Array(
							'assetids'          => Array($this->_running_vars['thesaurus_id'],),
							'lock_type'         => 'attributes',
							'dependants_only'   => false,
							'forceably_acquire' => false,
						);
	}//end getLockAssetDetails()


	/**
	* Get a reference to the thesaurus asset
	*
	* @access public
	* @return object     Thesaurus asset
	*/
	function &getThesaurusRef()
	{
		return $GLOBALS['SQ_SYSTEM']->am->getAsset($this->_running_vars['thesaurus_id']);

	}//end getThesaurusRef()

}//end class
?>
