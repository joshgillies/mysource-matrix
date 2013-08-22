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
* $Id: form.inc,v 1.281.2.3 2013/08/22 00:35:13 akarelia Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset.inc';
require_once SQ_LIB_PATH.'/html_form/html_form.inc';
require_once SQ_FUDGE_PATH.'/general/datetime.inc';
require_once SQ_FUDGE_PATH.'/general/general.inc';
require_once SQ_FUDGE_PATH.'/general/www.inc';
require_once SQ_CORE_PACKAGE_PATH.'/interfaces/bridge/bridge.inc';
require_once SQ_FUDGE_PATH.'/mollom/mollom.inc';


/**
* Form
*
* Purpose
*
*
* @author  Marc Mcintyre <mmcintyre@squiz.net>
* @version $Revision: 1.281.2.3 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Form extends Asset implements Bridge
{


	/**
	* the current answers to the form questions
	* @var Array
	*/
	var $current_answers = Array();


	/**
	* A form-level variable to store current "Extra Data" attached to its
	* questions
	*
	* Keyed by "questionid => array of Extra Data"
	*
	* @var Array
	*/
	var $extra_data = Array();


	/**
	* the active section object
	* @var object
	*/
	var $active_section = NULL;


	/**
	* the current submission's errors
	* @var object
	*/
	var $submission_errors = NULL;
	
	/**
	* the question validation errors
	* @var object
	*/
	var $question_errors = NULL;
	
	/**
	* the captcha validation errors
	* @var object
	*/
	var $captcha_errors = NULL;
	
	/**
	* the mollom validation errors
	* @var object
	*/
	var $mollom_errors = NULL;


	/**
	* the current submission's asset
	* @var object
	*/
	var $submission_asset = NULL;


	/**
	* the questions on the current page of the form
	* @var object
	*/
	var $current_page_questions = null;


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	* @return void
	* @access public
	*/
	function Form($assetid=0)
	{
		$this->_ser_attrs = TRUE;
		$this->Asset($assetid);

	}//end constructor



//--        GENERIC ASSET FUNCTIONS        --//


	/**
	* Create this asset, setting attributes prior to creation
	*
	* @param array	&$link	information used to create the initial link
	*
	* @return mixed int|boolean
	* @access public
	*/
	function create(&$link)
	{
		$this->setAttrValue('submission_errors_msg', translate('cms_form_error_msg_default'));

		return parent::create($link);

	}//end create()


	/**
	* Perform any additional processing required during the creation of this asset
	*
	* For example, creating other child assets should be performed in this method
	*
	* @param array	&$link	information used to create the initial link<br/>
	* <pre>
	* Array (
	*   'asset'        => [ref major asset to create link under],
	*   'link_type'    => SQ_LINK_?,
	*   'value'        => [link value],
	*   'sort_order'   => [link sort order],
	*   'is_dependant' => [0|1],
	*   'is_exclusive' => [0|1],
	* )
	* </pre>
	*
	* @return boolean
	* @access private
	*/
	function _createAdditional(&$link)
	{
		if (!$this->createSubmissionsFolder(translate('submissions'), 'submissions_folder')) {
			return FALSE;
		}

		return parent::_createAdditional($link);

	}//end _createAdditional()


	/**
	* Returns name of the asset
	*
	* @param boolean	$short_name	whether or not we are after the shortname or the full name
	* @param int		$contextid	what context to return the name from
	*
	* @return string
	* @access private
	* @see Asset::_getName()
	*/
	protected function _getName($short_name=FALSE, $contextid=NULL)
	{
		// No context specified, using the current context
		if ($contextid === NULL) {
			$contextid = $GLOBALS['SQ_SYSTEM']->getContextId();
		}//end if

		// Obtain the attribute value for Name from the specified Context
		$values = $GLOBALS['SQ_SYSTEM']->am->getAttributeValuesByName('name', $this->type(), Array($this->id), $contextid);
		if (empty($values) === TRUE) {
			return parent::_getName($short_name, $contextid);
		} else {
			return $values[$this->id];
		}

	}//end _getName()


	/**
	* Returns an array of all the permitted link type, the type asset and the cardinality
	*
	* @return array
	* @access private
	* @see Asset::_getAllowLinks()
	*/
	function _getAllowedLinks()
	{
		$page_links = Array(
						SQ_LINK_TYPE_2	=> Array(
											'form_section'	=> Array(
																'card'		=> 'M',
																'exclusive'	=> FALSE,
															   ),
											'folder'		=> Array(
																'card'		=> '3',
																'exclusive'	=> TRUE,
															   ),
										   ),
					  );
		return $page_links;

	}//end _getAllowedLinks()


	/**
	* Allow any required processesing to occur when any link is updated for the asset
	*
	* Bug Fix #1671
	* When a new link to the section is created or deleted, update the content file
	*
	* @return void
	* @access protected
	*/
	function linksUpdated()
	{
		$this->fileRegeneration();

	}//end linksUpdated()



	/**
	* Clones certain specified components of the asset
	*
	* @param object		&$clone		the clone asset shell
	* @param array		$components	the wanted components to clone eg.
	* <pre>
	* 								Array(
	* 									'attributes',
	* 									'metadata_schemas',
	* 									'metadata',
	* 									'workflow',
	* 									'permissions',
	* 									'data',
	* 									'content_tags',
	* 									'roles',
	* 								);
	* 								or alternately
	* 								Array('all');
	* </pre>
	* @param boolean	$override	whether or not to override the existing permission, metadata schemas, workflow schemas with the new ones.
	*
	* @return boolean
	* @access public
	* @see asset::cloneComponents()
	*/
	function cloneComponents(&$clone, $components, $override=FALSE)
	{
		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		if (!parent::cloneComponents($clone, $components, $override)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return FALSE;
		}

		if (in_array('attributes', $components) || in_array('all', $components)) {
			$sort_order = $this->attr('sort_order');
			$sort_order = str_replace($this->id.':q', $clone->id.':q', $sort_order);

			// save the information
			$clone->setAttrValue('sort_order', $sort_order);
			$clone->saveAttributes();

			// Change the content of the content type to replace question id.

		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
		return TRUE;

	}//end cloneComponents()


	/**
	* Perform any additional processing required during the cloning of this asset
	*
	* This function is being called by asset_manager after the dependants of asset
	* have been cloned.
	*
	* @param object	&$clone		the clone asset shell
	* @param array	$components	the wanted components to clone eg.
	*							Array (
	*							'attributes',
	*							'metadata_schemas',
	*							'metadata',
	*							'workflow',
	*							'permissions',
	*							'data',
	*							);
	*							or alternately
	*							Array('all');
	*
	* @return boolean
	* @access public
	* @see asset::cloneComponentsAdditional()
	*/
	function cloneComponentsAdditional(&$clone, $components)
	{

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		if (in_array('attributes', $components) || in_array('all', $components)) {
			$GLOBALS['SQ_SYSTEM']->setRunLevel($GLOBALS['SQ_SYSTEM']->getRunLevel() & SQ_RUN_LEVEL_FORCED);


			$section_links = $this->getSectionLinks();
			$clone_section_links = $clone->getSectionLinks();

			$originals_clones	= Array();
			foreach ($section_links as $link) {
				foreach ($clone_section_links as $cloned_link) {
					if ($cloned_link['sort_order'] == $link['sort_order']) {
						// Create a new array mapping original id and cloned id of sections.
						$originals_clones[]	= Array (
												'original'	=> $link['minorid'],
												'clone'		=> $cloned_link['minorid'],
											  );
					}
				}
			}

			// Might as well add this form and its clone to the list to do.
			$originals_clones[]	= Array (
									'original'	=> $this->id,
									'clone'		=> $clone->id,
								  );

			// Now get all the bodycopies, and get their content types
			$bodycopies	= Array (
							'use_bodycopy_main'			=> 'Page Contents',
							'use_bodycopy_thank_you'	=> 'Thank You',
						  );
			if ((isset($clone->_bodycopies) && !empty($clone->_bodycopies)) ) {
				$bodycopies = array_merge($bodycopies, Array ('use_bodycopy_receipt' => 'Receipt'));
			}

			$replace_keywords = Array ( 'question_name', 'response', 'question_field', 'question_id', 'question_note', 'question_label', 'section_title');
			foreach ($bodycopies as $attribute => $bodycopy_name) {
				$bodycopy = NULL;
				if ((isset($clone->_bodycopies) && !empty($clone->_bodycopies)) ) {
					$bodycopy = $clone->getBodycopy($bodycopy_name);
				} else {
					$attr_value = $clone->attr($attribute);
					if (!empty($attr_value)) $bodycopy = $clone->getBodycopy($bodycopy_name);
				}

				if (!empty($bodycopy)) {
					$containers = $bodycopy->getContainers();
					foreach ($containers as $container) {
						$content_type = $container->getContentType();
						if ($container->type() == 'bodycopy_table') {
							if (is_array($content_type)) {
								foreach ($content_type as $cell_type) {
									if (!$this->_cloneDIVContents($cell_type, $originals_clones, $replace_keywords)) {
										$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
										$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
										return FALSE;
									}//end if
								}//end foreach
							} else {
								if (!$this->_cloneDIVContents($content_type, $originals_clones, $replace_keywords)) {
									$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
									$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
									return FALSE;
								}//end if
							}//end if
						} else {
							if (!$this->_cloneDIVContents($content_type, $originals_clones, $replace_keywords)) {
								$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
								$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
								return FALSE;
							}//end if
						}//end if
					}//end foreach
				}//end if
			}//end foreach

			$GLOBALS['SQ_SYSTEM']->restoreRunLevel();

		}//end if components

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
		return TRUE;

	}//end cloneComponentsAdditional()


	/**
	* Clone the contents div assets
	*
	* @param object	$content_type		The content type asset to be cloned
	* @param array	$original_clones	An array containing the assetids of the original and cloned assets
	* @param array	$replace_keywords	An array containing the keyword of the assets
	*
	* @return boolean
	* @access private
	*/
	function _cloneDIVContents($content_type, $original_clones=Array(), $replace_keywords=Array())
	{
		$old_html = $content_type->attr('html');
		if (!empty($old_html) && !empty($original_clones)) {
			$new_html = $content_type->attr('html');
			foreach ($original_clones as $original_clone) {
				// Replace the old question id with the new question id
				foreach ($replace_keywords as $keyword) {
					if ($keyword == 'section_title') {
						$new_html = str_replace($keyword.'_'.$original_clone['original'], $keyword.'_'.$original_clone['clone'], $new_html);
					} else {
						$new_html = str_replace($keyword.'_'.$original_clone['original'].'_q', $keyword.'_'.$original_clone['clone'].'_q', $new_html);
					}
				}//end foreach
			}//end foreach

			// Save and check for errors, the rollback occurs above
			if (!$content_type->setAttrValue('html', $new_html)) {
				return FALSE;
			}//end if

			if (!$content_type->saveAttributes()) return FALSE;

		}//end if

		return TRUE;

	}//end _cloneDIVContents()


//--        BRIDGE FUNCTIONS        --//


	/**
	* Returns a reference to the asset represented by the passed assetid (or NULL on error)
	*
	* @param int		$assetid		the full asset ID to be loaded
	* @param string		$type_code		if this exists then this object is used to
	*									load the asset, if not then the DB is queried
	*									to find out the asset type
	* @param boolean	$mute_errors	stops the outputting of errors in this fn, needed because
	*									you can't use the '@' operator when returning by reference
	*
	* @return object
	* @access public
	* @see Asset_Manager::&getAsset()
	*/
	public function getAsset($assetid, $type_code='', $mute_errors=FALSE)
	{
		$asset = NULL;
		$id_parts = explode(':', $assetid);

		if (isset($id_parts[1])) {
			$shadowid = $id_parts[1];
		} else {
			return $asset;
		}

		// Questions follow the format '<assetid>:q<shadowid>'
		if ($shadowid{0} == 'q') {

			// pick up the question in question (!)
			$questions = $this->attr('questions');
			$questionid = substr($shadowid, 1, strlen($shadowid));
			if (empty($questions[$questionid])) return $asset;

			$q_type_code = $questions[$questionid]['question_type_code'];

			// not fussed about type code?
			if (empty($type_code)) $type_code = $q_type_code;

			// only give the asset back if of the right type code
			if (in_array($q_type_code, $GLOBALS['SQ_SYSTEM']->am->getTypeDescendants($type_code, TRUE))) {
				$questions[$questionid]['questionid'] = $questionid;
				$GLOBALS['SQ_SYSTEM']->am->includeAsset($q_type_code);

				// this eval returns the necessary object for the question type
				$asset = new $q_type_code($this->id, $questions[$questionid]);
			}
		}

		return $asset;

	}//end getAsset()


	/**
	* Return all links that this asset has to any shadow assets
	*
	* @param int			$assetid			id of the the $side_of_link asset
	* @param int			$link_types			integer that can be the product of bitwise operations
	*											on the SQ_LINK_* constants
	* @param string|array	$type_code			the type of asset that is linked
	*											(eg 'Page', 'File', etc)
	*											if an array returns link if matches any
	*											of the array values
	* @param boolean		$strict_type_code	whether we are finding assets that are just a $type_code
	*											or $type_code and any of it's sub-classes
	* @param string			$side_of_link		Which side of the link this (the current) asset
	*											is on ('major' or 'minor')
	* @param string			$sort_by			a field in the asset table you would like the results
	*											sorted by (eg. name, short_name etc)
	*
	* @return array
	* @access public
	*/
	function getLinks($assetid, $link_types, $type_code='', $strict_type_code=TRUE, $side_of_link='major', $sort_by=NULL)
	{
		$id_parts = explode(':', $assetid);
		if (isset($id_parts[1])) $assetid = $id_parts[1];

		$links = Array();
		$new_sort_order = 0;

		// shadow assets have no children, but they have one minor link the other way
		if (!is_numeric($assetid)) {
			if ($side_of_link == 'minor') {
				$return_link = FALSE;

				if ($type_code == '') {
					// not fussed what we're getting, so return me
					$return_link = TRUE;
				} else if ($strict_type_code) {
					// strict type code check
					$return_link = ($type_code == $this->type());
				} else {
					$return_link = (in_array($this->type(), $GLOBALS['SQ_SYSTEM']->am->getTypeDescendants($type_code, TRUE)));
				}

				if ($return_link) {
					$links[] = Array(
								'linkid'			=> 0,
								'majorid'			=> $this->id,
								'minorid'			=> $this->id.':'.$assetid,
								'major_type_code'	=> $this->type(),
								'value'				=> '',
								'link_type'			=> SQ_LINK_TYPE_2,
								'is_dependant'		=> TRUE,
								'is_exclusive'		=> FALSE,
								'sort_order'		=> 0,
								'locked'			=> 0,
							   );
				}
			}

			return $links;
		}

		// are we getting our bodycopy folder and/or sections (if they all exist)?
		$real_type_codes = Array(
							'folder'		=> SQ_LINK_TYPE_2 | SQ_LINK_TYPE_3,
							'form_section'	=> SQ_LINK_TYPE_2,
						   );

		// asset_manager will handle all the real links for us, but we need to determine sort order;
		// get all the links, but don't add them to the links array
		foreach ($real_type_codes as $real_type_code => $allowed_link_types) {
			if ($type_code == '') {
				// not fussed what we're getting, so return me
				$get_type = TRUE;
			} else if ($strict_type_code) {
				$get_type = ($type_code == $real_type_code);
			} else {
				// specific type code set, non-strict check
				$get_type = (in_array($real_type_code, $GLOBALS['SQ_SYSTEM']->am->getTypeDescendants($type_code, TRUE)));
			}

			// we are getting this type?
			if ($get_type) {
				$query = $GLOBALS['SQ_SYSTEM']->am->generateGetLinksQuery($assetid, $link_types & $allowed_link_types, $real_type_code, FALSE);
				if (empty($query)) return Array();

				$db = MatrixDAL::getDb();
				try {
					$stmt = MatrixDAL::preparePdoQuery(implode(' ', $query['sql_array']));
					foreach ($query['bind_vars'] as $bind_var => $bind_value) {
						MatrixDAL::bindValueToPdo($stmt, $bind_var, $bind_value);
					}
					$type_links = MatrixDAL::executePdoGroupedAssoc($stmt);
				} catch (Exception $e) {
					throw new Exception('Unable to get links for asset: '.$assetid.' due to database error: '.$e->getMessage());
				}

				foreach (array_keys($type_links) as $key) {
					$new_sort_order++;
				}
			}
		}

		// but we also need question shadow asset links
		if ($link_types & SQ_LINK_TYPE_2) {
			$questions = $this->attr('questions');

			// keep a cache of asset type codes if we are checking on a non-strict
			// type code, to stop us calling getTypeDescendants() all the time
			// (not needed if strict type check or if not fussed)
			$get_question_types = Array();

			foreach ($questions as $questionid => $data) {
				if ($type_code == '') {
					// not fussed what we're getting, so return me
					$get_question = TRUE;
				} else if ($strict_type_code) {
					// strict type code check
					$get_question = ($type_code == $data['question_type_code']);
				} else {
					// if we've already cached whether this question type is to be
					// returned, then we don't need to look it up again
					if (isset($get_question_types[$data['question_type_code']])) {
						$get_question = $get_question_types[$data['question_type_code']];
						$get_question_types[$data['question_type_code']] = $get_question;
					} else {
						$get_question = (in_array($data['question_type_code'], $GLOBALS['SQ_SYSTEM']->am->getTypeDescendants($type_code, TRUE)));
					}
				}

				if ($get_question) {
					$links[] = Array(
								'linkid'			=> 0,
								'majorid'			=> $this->id,
								'minorid'			=> $this->id.':q'.$questionid,
								'minor_type_code'	=> $data['question_type_code'],
								'value'				=> '',
								'link_type'			=> SQ_LINK_TYPE_2,
								'is_dependant'		=> TRUE,
								'is_exclusive'		=> FALSE,
								'sort_order'		=> $new_sort_order,
								'locked'			=> 0,
							   );
				}
				$new_sort_order++;
			}//end foreach
		}//end if

		if (is_null($sort_by)) $sort_by = 'sort_order';

		uasort($links, create_function('$a,$b','return $a["'.$sort_by.'"] > $b["'.$sort_by.'"];'));

		return $links;

	}//end getLinks()


	/**
	* Used by asset map to return a asset map-style link array
	*
	* @return array
	* @access public
	*/
	function getAssetMapLinks()
	{
		$new_sort_order = 0;
		$old_links = $this->getLinks($this->id, SQ_SC_LINK_BACKEND_NAV);
		$links = Array();
		while (!empty($old_links)) {
			array_push($links, array_shift($old_links));
		}

		$questions = $this->attr('questions');

		foreach (array_keys($links) as $i) {

			$link =& $links[$i];

			// remove real links
			if ($link['minor_type_code'] == 'folder') {
				unset($links[$i]);
				continue;
			}

			if ($link['minor_type_code'] == 'form_section') {
				unset($links[$i]);
				continue;
			}

			// mould it all to the asset map's liking
			$link['url'] = '';
			$link['path'] = '';
			$link['num_kids'] = 0;
			$link['accessible'] = 1;

			$link['assetid'] = $link['minorid'];
			$link['type_code'] = $link['minor_type_code'];
			$link['linkid'] = $link['majorid'].':'.$link['minorid'];

			$questionid = str_replace($this->id.':q', '', $link['assetid']);

			// make name and short name the same
			$link['name'] = $questions[$questionid]['attributes']['name'];
			$link['short_name'] = $link['name'];

			// make the status the same as the form's one
			$link['status'] = $this->status;

			unset($link['minor_type_code']);
			unset($link['majorid']);
			unset($link['minorid']);
			unset($link['value']);
			unset($link['is_exclusive']);

		}//end foreach

		return $links;

	}//end getAssetMapLinks()


	/**
	* Get all asset ids that are above the passed assetid in the various trees in which it exists
	*
	* @param int			$shadowid			the shadow ID of the shadow asset to get (ie. the
	*											part after the ':')
	* @param string|array	$type_code			the type of asset that is linked (eg 'User', 'User_Group', etc)
	*											if an array returns link if matches any of the array values
	* @param boolean		$strict_type_code	whether we are finding assets that are just a $type_code
	*											or $type_code and any of it's sub-classes
	*
	* @return array
	* @access public
	* @see Asset_Manager::getParents()
	*/
	function getParents($shadowid, $type_code='', $strict_type_code=TRUE)
	{
		$id_parts = explode(':', $shadowid);
		if (isset($id_parts[1])) $shadowid = $id_parts[1];

		// basically get the parents of the section, and add itself in the
		$ret_val = $GLOBALS['SQ_SYSTEM']->am->generateGetParentsQuery($this->id, $type_code, $strict_type_code);

		if (empty($ret_val)) return Array();

		try {
			$query = MatrixDAL::preparePdoQuery(implode(' ', $ret_val['sql_array']));
			foreach ($ret_val['bind_vars'] as $bind_var => $bind_value) {
				MatrixDAL::bindValueToPdo($query, $bind_var, $bind_value);
			}

			$queried_parents = MatrixDAL::executePdoAssoc($query);
		} catch (Exception $e) {
			throw new Exception('Unable to get the parents of shadow id: '.$shadowid.' due to database error: '.$e->getMessage());
		}
		$parents = Array();

		foreach ($queried_parents as $queried_parent) {
			$parents[$queried_parent['majorid']] = $queried_parent['type_code'];
		}

		$parents[$this->id] = $this->type();

		return $parents;

	}//end getParents()


	/**
	* Get all asset ids that are below the passed assetid in the various trees in which it exists
	*
	* Return structure: Array(int => string) - asset id => type_code
	*
	* @param int			$assetid			the id of the asset to get its children for
	* @param string|array	$type_code			the type of asset that is linked
	*											(eg 'Page', 'File', etc)
	*											if an array returns link if matches any
	*											of the array values
	* @param boolean		$strict_type_code	whether we are finding assets that are just a $type_code
	*											or $type_code and any of it's sub-classes
	* @param boolean		$dependant			if TRUE, results will be filtered to assets that are dependants.
	*											If FALSE, results will all not be dependants.
	*											If NULL, results will not be filtered
	* @param string			$sort_by			a field in the asset table you would like the results
	*											sorted by (eg. name, short_name etc)
	*
	* @return array
	* @access public
	* @see Asset_Manager::getChildren()
	*/
	function getChildren($assetid, $type_code='', $strict_type_code=TRUE, $dependant=NULL, $sort_by=NULL)
	{
		// no shadow assets have children in this asset
		if (!is_numeric($assetid)) return Array();

		if (!is_array($type_code)) {
			if (empty($type_code)) {
				$type_code = Array();
			} else {
				$type_code = Array($type_code);
			}
		}

		$children = Array();

		// this is so we get the sections as we go
		// have to do this all in here because calling asset manager will bring on a vicious loop
		$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
		if (($asset instanceof Bridge) && ($assetid != $this->id)) {
			$children = $asset->getChildren($assetid, $type_code, $dependant, $strict_type_code, $sort_by);
			$this->forgetAsset($asset);
		} else {
			$ret_val = $GLOBALS['SQ_SYSTEM']->am->generateGetChildrenQuery($asset, $type_code, $strict_type_code, $dependant, $sort_by);
			unset($asset);

			if (!empty($ret_val)) {
				try  {
					$sql_array = $ret_val['sql_array'];
					$bind_vars = $ret_val['bind_vars'];

					$query = MatrixDAL::preparePdoQuery(implode(' ', $sql_array));
					foreach ($bind_vars as $bind_var => $bind_value) {
						MatrixDAL::bindValueToPdo($query, $bind_var, $bind_value);
					}
					$result = MatrixDAL::executePdoGroupedAssoc($query);
				} catch (Exception $e) {
					throw new Exception('Unable to get children for asset: '.$assetid.' due to database error: '.$e->getMessage());
				}

				$children =& $result;
			}
		}

		// now questions
		$entries = $this->attr('questions');
		if (empty($type_code)) {
			foreach ($entries as $questionid => $data) {
				$children[$this->id.':q'.$questionid] = Array(Array('type_code' => $data['question_type_code']));
			}
		} else {
			foreach ($type_code as $this_type_code) {
				foreach ($entries as $questionid => $data) {
					if ($strict_type_code) {
						if ($data['question_type_code'] == $this_type_code) {
							$children[$this->id.':q'.$questionid] = Array(Array('type_code' => $data['question_type_code']));
						}
					} else {
						$type_desc = $GLOBALS['SQ_SYSTEM']->am->getTypeDescendants($this_type_code) + Array($this_type_code);
						if (in_array($data['question_type_code'], $type_desc)) {
							$children[$this->id.':q'.$questionid] = Array(Array('type_code' => $data['question_type_code']));
						}
					}
				}
			}
		}

		foreach ($children as $child_id => $details) {
           	foreach ($type_code as $index => $code) {
				if ($details[0]['type_code'] != $code) {
					unset($children[$child_id]);
				}
			}
        }

		if (!is_null($sort_by)) {
			uasort($children, create_function('$a,$b','return $a["'.$sort_by.'"] > $b["'.$sort_by.'"]'));
		}

		return $children;

	}//end getChildren()


	/**
	* Set the last updated info for this asset
	*
	* Call this function when the asset has been changed in some way so we can indicated in the
	* asset table when the asset was last changed and who by. This function will also increment the micro
	* version number for the asset.
	*
	* @param boolean	$update_parents	should we go through an update all our dependant parents as well ?
	*									NOTE: this is passed to incrementVersion to let it know that the parents
	*									should have their versions updated as well.
	*
	* @return boolean
	* @access private
	*/
	function _updated($update_parents=TRUE)
	{
		if (!parent::_updated($update_parents)) return FALSE;
		return $this->fileRegeneration();

	}//end _updated()


	/**
	* Returns TRUE if the passed minor asset can be linked to this asset, or a string with the error msg
	*
	* @param object	&$minor		the minor asset that we are linking to
	* @param string	$link_type	the type of link this is
	* @param int	$exclusive	the exclusive status of the link (gets passed to canLinkToType)
	*
	* @return mixed boolean|string
	* @access public
	*/
	function canCreateLink(&$minor, $link_type, $exclusive)
	{
		if (($minor instanceof Form_Submission) || ($minor instanceof Form_Section)) {
			return TRUE;
		}

		return parent::canCreateLink($minor, $link_type, $exclusive);

	}//end canCreateLink()


	/**
	* Prepares for linking by checking that the link values are valid for this link
	*
	* This function will be called if this asset is the major or minor party in the link, so the side_of_link flag
	* should be checked to work out what side of the link this asset is on. The return value should indicate
	* if any of the link fields were changed.
	*
	* @param object	&$asset			the major or minor asset that we are linking to
	* @param string	$side_of_link	the side of the link we are on (major or minor)
	* @param string	&$link_type		the type of link we are creating
	* @param string	&$value			the value that is to be associated with the link
	* @param string	&$sort_order	the position in the links list that this link should take,
	* @param string	&$dependant		'0' / '1' on whether the this asset is dependant on the
	*								asset that will be linked by the new link
	* @param string	&$exclusive		'0' / '1' on whether the this asset is linked exclusivly
	*								to the asset that will be linked by the new link
	*
	* @return boolean
	* @access public
	*/
	function prepareLink(&$asset, $side_of_link, &$link_type, &$value, &$sort_order, &$dependant, &$exclusive)
	{
		if ($side_of_link == 'major' && $asset->type() == 'form_section') {
			$link_type = SQ_LINK_TYPE_2;
			$dependant = TRUE;
		}

	}//end prepareLink()


	/**
	* Return the number of links in the system that involve the specified asset
	*
	* Can be restricted by either link type and/or asset type code
	*
	* @param int		$assetid			id of the the $side_of_link asset
	* @param string		$side_of_link		Which side of the link this (the current) asset
	*										is on ('major' or 'minor')
	* @param int		$link_types			integer that can be the product of bitwise operations
	*										on the SQ_LINK_* constants
	* @param mixed		$type_code			the type of asset that is linked (eg 'Page', 'File', etc)
	*										if an array returns link if matches any
	*										of the array values
	* @param boolean	$strict_type_code	whether we are finding assets that are just a $type_code
	*										or $type_code and any of it's sub-classes
	* @param int		$ignore_linkid		ignore the link represented by this link
	*										id when returning the count
	*
	* @return int
	* @access public
	*/
	function countLinks($assetid, $side_of_link='major', $link_types=0, $type_code='', $strict_type_code=TRUE, $ignore_linkid=0)
	{
		return 0;

	}//end countLinks()



	/**
	* Returns an array of assetid's + info in the order that they propogate out from THIS BRIDGE'S url
	*
	* @param string	$assetid	the id of the last asset in the lineage
	* @param string	$protocol	the protocol to match -> null means it is ignored
	* @param string	$url		the url to check for -> null defaults it to current url
	*
	* @return array
	* @access public
	*/
	function getLineageFromURL($assetid, $protocol, $url)
	{
		return Array();

	}//end getLineageFromURL()


	/**
	* Returns an array of all user or user_group assets that have the passed permission for the passed asset
	*
	* @param int		$assetid		the assetid for asset whose permissions to get
	* @param string		$permission		the permission code you are getting
	* @param boolean	$granted		type of Access : null = all, TRUE = granted, FALSE = denied
	* @param boolean	$and_greater	get effective permission (eg read access = read
	*									or write or admin)
	* @param boolean	$expand_groups	expand user groups so only user ids get returned
	*									(NOTE: only valid if $all_info is FALSE)
	* @param boolean	$all_info		when FALSE fn just returns an array of userids for
	*									those that have permission
	*									When TRUE fn returns all info about the permission in the form of
	*									Array(userid => granted)
	*									(NOTE: TRUE is only valid if $and_greater and
	*									$expand_groups are FALSE)
	*
	* @return array
	* @access public
	*/
	function getPermission($assetid, $permission, $granted=NULL, $and_greater=TRUE, $expand_groups=FALSE, $all_info=FALSE)
	{
		return Array();

	}//end getPermission()


	/**
	* Sets a permission for the passed user or user_group
	*
	* @param int		$assetid	the assetid for asset whose permissions to set
	* @param int		$userid		the id of the user or user_group to add the permission for
	* @param string		$permission	the permission code you are adding
	* @param boolean	$granted	whether this permission grants access or not
	*
	* @return boolean
	* @access public
	*/
	function setPermission($assetid, $userid, $permission, $granted)
	{
		return FALSE;

	}//end setPermission()


	/**
	* Deletes a permission with the passed user or user_group
	*
	* @param int	$assetid	the assetid for asset whose permissions to delete
	* @param int	$userid		the id of the user or user_group to remove the permission from
	* @param string	$permission	the permission code you are deleting
	*
	* @return boolean
	* @access public
	*/
	function deletePermission($assetid, $userid, $permission)
	{
		return FALSE;

	}//end deletePermission()


	/**
	* Returns an array of information about assets of the passed codes
	*
	* If one value in $field then the return value is Array(assetid => '[field]'), otherwise Array(assetid => Array())
	*
	* @param array		$assetids			an array of assets to limit
	* @param mixed		$type_code			an asset type code, or array of typecodes, to limit results to
	* @param boolean	$strict_type_code	ensure returned assets are of the passed type codes
	* @param string		$field				single field from the asset table to return (along with the assetid)
	*										Default is all fields
	*
	* @return array
	* @access public
	*/
	function getAssetInfo($assetids, $type_code=Array(), $strict_type_code=TRUE, $field='')
	{
		return Array();

	}//end getAssetInfo()


	/**
	* Determine if the passed assetid or assetids refer(s) to a valid asset in the system
	*
	* Note that this will always return FALSE if you pass it a shadow asset
	*
	* @param mixed	$assetids	the ID of the asset we are going to the check for existence
	*							or an array of assetids we want to check
	*
	* @return mixed boolean|array
	* @access public
	*/
	function assetExists($assetids)
	{
		return FALSE;

	}//end assetExists()


	/**
	* Get details of a link with the specified characteristics
	*
	* @param int		$assetid			id of the the $side_of_link asset
	* @param int		$link_type			integer that should be a single integer of the SQ_LINK_* constants
	* @param mixed		$type_code			the type of asset that is linked
	*										(eg 'Page', 'File', etc)
	*										if an array returns link if matches any
	*										of the array values
	* @param boolean	$strict_type_code	whether we are finding an asset that
	*										is just a $type_code or potentially an inherited type
	*										or $type_code and any of it's sub-classes
	* @param string		$value				the value that is associated with this link
	* @param string		$side_of_link		Which side of the link this (the current) asset
	*										is on ('major' or 'minor')
	* @param boolean	$exclusive			The exclusive status for the link must be
	*										this (if not null)
	*
	* @return array
	* @access public
	*/
	function getLink($assetid, $link_type=NULL, $type_code='', $strict_type_code=TRUE, $value=NULL, $side_of_link='major', $exclusive=NULL)
	{
		return Array();

	}//end getLink()


	/**
	* Get details of the link with specified linkid
	*
	* @param int	$linkid			id of the link were returning
	* @param int	$assetid		id of the the $side_of_link asset
	*								if zero no check is made and both major and minor information is returned
	* @param string	$side_of_link	Which side of the link the asset represented by $assetid is on ('major' or 'minor')
	*
	* @return array
	* @access public
	*/
	/**
	* Get details of the link with specified linkid
	*
	* @param int	$linkid			id of the link were returning
	* @param int	$assetid		id of the the $side_of_link asset
	*								if zero no check is made and both major and minor information is returned
	* @param string	$side_of_link	Which side of the link the asset represented by $assetid is on ('major' or 'minor')
	*
	* @return array
	* @access public
	*/
	function getLinkById($linkid, $assetid=0, $side_of_link='major')
	{
		if (!is_numeric($assetid)) {
			if ($side_of_link == 'minor') {
				return Array(
						'linkid'			=> 0,
						'majorid'			=> $this->id,
						'minorid'			=> $this->id.':'.$assetid,
						'major_type_code'	=> $this->type(),
						'value'				=> '',
						'link_type'			=> SQ_LINK_TYPE_2,
						'is_dependant'		=> TRUE,
						'is_exclusive'		=> FALSE,
						'sort_order'		=> 0,
						'locked'			=> 0,
					   );
			}
			return Array();
		}

		$id_parts = explode(':', $linkid);
		if (isset($id_parts[1]) && isset($id_parts[2])) {
			$looking_for = $id_parts[1].':'.$id_parts[2];
			$tmp_question_nb = str_replace('q', '', $id_parts[2]);
			$questions = $this->attr('questions');
			if (isset($questions[$tmp_question_nb])) {
				return Array(
						'linkid'			=> $linkid,
						'majorid'			=> $this->id,
						'minorid'			=> $this->id.':q'.$tmp_question_nb,
						'minor_type_code'	=> $questions[$tmp_question_nb]['question_type_code'],
						'value'				=> '',
						'link_type'			=> SQ_LINK_TYPE_2,
						'is_dependant'		=> TRUE,
						'is_exclusive'		=> FALSE,
						'sort_order'		=> 0,
						'locked'			=> 0,
					   );
			}
		}

		return Array();

	}//end getLinkById()


	/**
	* Get details of the link(s) between the two specified assets
	*
	* @param int		$assetid		id of the the $side_of_link asset
	* @param int		$other_assetid	the asset on the other side of the link
	* @param int		$link_types		integer that can be the product of bitwise operations
	*									on the SQ_LINK_* constants
	* @param string		$value			the value that is associated with this link
	* @param string		$side_of_link	Which side of the link the first assetid is on
	*									('major' or 'minor')
	* @param boolean	$force_array	force the results to return an array of links
	*									even if there is only one link
	* @param boolean	$dependant		The dependant status for all the links must be
	*									this (if not null)
	* @param boolean	$exclusive		The exclusive status for all the links must be
	*									this (if not null)
	*
	* @return array
	* @access public
	*/
	function getLinkByAsset($assetid, $other_assetid, $link_types=NULL, $value=NULL, $side_of_link='major', $force_array=FALSE, $dependant=NULL, $exclusive=NULL)
	{
		return Array();

	}//end getLinkByAsset()


	/**
	* Get details of the links from the specified asset to all its children
	*
	* @param string	$assetid	the assetid of the asset to find the child links for
	* @param int	$link_type	the type of links to get
	*
	* @return array
	* @access public
	*/
	function getAllChildLinks($assetid, $link_type=0)
	{
		return Array();

	}//end getAllChildLinks()


	/**
	* Update the details of an existing link
	*
	* If any of the detail vars are NULL they are not updated.
	*
	* @param int	$linkid		the link id of the link to update
	* @param int	$link_type	the type of the link (one of the SQ_LINK_* constants)
	* @param string	$value		the value to place on the link
	* @param int	$sort_order	the position in the links list that this link should take,
	*							if less than zero places at end of list
	*
	* @return boolean
	* @access public
	*/
	function updateLink($linkid, $link_type=NULL, $value=NULL, $sort_order=NULL)
	{
		return FALSE;

	}//end updateLink()


	/**
	* Remove a link by id
	*
	* @param int		$linkid	the link id of the link to remove
	* @param boolean	$moving	TRUE if this delete is part of a move operation
	*
	* @return boolean
	* @access public
	*/
	function deleteAssetLink($linkid, $moving=FALSE)
	{
		$id_parts = explode(':', $linkid);
		if (isset($id_parts[1]) && isset($id_parts[2])) {
			$question_to_delete = $id_parts[1].':'.$id_parts[2];
			$asset_to_delete = $this->getAsset($question_to_delete);
			if ($asset_to_delete != NULL) {
				return $this->deleteQuestion($asset_to_delete);
			}
		}

		return FALSE;

	}//end deleteAssetLink()


	/**
	* Get asset info for use by asset map
	*
	* @param string	$assetid	Full Asset id to get information for
	*
	* @return array
	* @access public
	*/
	function getAssetMapAssetInfo($assetid)
	{
		return Array();

	}//end getAssetMapAssetInfo()


	/**
	* Create a link between the two supplied assets
	*
	* @param object		&$major		the major asset that we are linking from
	* @param object		&$minor		the minor asset that we are linking to
	* @param string		$link_type	the type of link this is
	* @param string		$value		the value that is to be associated with this link
	* @param string		$sort_order	the position in the links list that this link should take,
	*								if null or less than zero places at end of list
	* @param string		$dependant	'0' / '1' - whether the minor asset is dependant on the major
	* @param string		$exclusive	'0' / '1' - whether the major asset is to be the minor's only parent
	* @param boolean	$moving		whether or not this asset is being moved
	*
	* @return int
	* @access public
	*/
	function createAssetLink(&$major, &$minor, $link_type, $value='', $sort_order=NULL, $dependant='0', $exclusive='0', $moving=FALSE)
	{
		return 0;

	}//end createAssetLink()


//--        QUESTION MANAGEMENT        --//


	/**
	* Get the question links attached to this form (and only to this form).
	*
	* Returns an array of (question ID => question detail).
	*
	* @return array
	* @access public
	*/
	function &getQuestions()
	{
		$questions = $this->attr('questions');
		return $questions;

	}//end getQuestions()


	/**
	* get the assetids of ALL of this form's questions, including those
	* inside sections (even nested sections).
	*
	* Returns an array of (full assetid => question detail)
	*
	* @return array
	* @access public
	*/
	function &getAllQuestions()
	{
		// getQuestions() returns an array of (q_id => detail) - this is
		// ambiguous to return if sections are present, so we need to convert
		// to an asset ID

		$all_questions = Array();
		$questions = $this->getQuestions();
		foreach ($questions as $q_id => $question_detail) {
			$all_questions[$this->id.':q'.$q_id] = $question_detail;
		}

		$sections = $this->getAllSections();
		foreach ($sections as $section) {
			$questions = $section->getQuestions();
			foreach ($questions as $q_id => $question_detail) {
				$all_questions[$section->id.':q'.$q_id] = $question_detail;
			}
		}

		return $all_questions;

	}//end getAllQuestions()


	/**
	* Gets only the question assets that are attached to this form.
	*
	* @param string	$type_code	When specified, only return assets of this exact type
	*
	* @return array
	* @access public
	*/
	function &getQuestionAssets($type_code=NULL)
	{
		$am = $GLOBALS['SQ_SYSTEM']->am;
		$result = Array();

		$questions = $this->attr('questions');
		if (!empty($questions)) {
			foreach ($questions as $q_id => $question) {
				if (empty($type_code) || ($question['question_type_code'] === $type_code)) {
					$id           =  $this->id.':q'.$q_id;
					$result[$id]  = $am->getAsset($id);
				}
			}
		}

		return $result;

	}//end getQuestionAssets()


	/**
	* get the question assets attached to this form, even inside sections
	*
	* This also includes all questions inside nested sections.
	*
	* @param string	$type_code	When specified, only return assets of this exact type
	*
	* @return array
	* @access public
	*/
	function &getAllQuestionAssets($type_code=NULL)
	{
		$all_questions = $this->getQuestionAssets($type_code);

		$sections = $this->getAllSections();
		foreach ($sections as $section) {
			$questions = $section->getQuestionAssets($type_code);
			$all_questions = array_merge($all_questions, $questions);
		}

		return $all_questions;

	}//end getAllQuestionAssets()


	/**
	* Get questions attached to certain page
	*
	* @param int	$page_number	The page number to get questions from
	*
	* @return array
	*/
	public function getQuestionAssetsOnPage($page_number)
	{
		// If we are on the last page of a form with a confirmation page, then
		// we must be on the confirmation page, which by definition has no
		// answerable questions (possibly a CAPTCHA but this doesn't count).
		if (($page_number == $this->getTotalPages()) && $this->attr('use_confirmation_page')) {
			return Array();
		}

		if ($this->attr('multi_page')) {
			// do we have any unattached questions?
			$unattached_questions = $this->getQuestions();

			// Get top-level sections
			$section_links = $this->getSectionLinks();

			// If we have unattached questions, then reduce the page number
			// internally - so that section 1 will be on page 2
			if (!empty($unattached_questions)) {
				$page_number--;
			}

			if ($page_number == 0) {
				// Printing unattached questions (if any)
				// Grab the default "unattached questions" content file (at the moment)
				$questions = $this->getQuestionAssets();
			} else {
				// Printing a section - best way to do this is to slice it off
				$section_link = reset(array_slice($section_links, $page_number - 1, 1));
				$section = $GLOBALS['SQ_SYSTEM']->am->getAsset($section_link['minorid']);
				$questions = $section->getAllQuestionAssets();
			}
		} else {
			// Single page form - get everything
			$questions = $this->getAllQuestionAssets();
		}

		return $questions;

	}//end getAllQuestionAssets()


//--        ADDING AND DELETING QUESTIONS        --//


	/**
	* attach a question to this form
	*
	* Returns boolean FALSE if a database problem occurs or if question already
	* attached, or TRUE if sucessful
	*
	* @param string	$type_code	the type code of the question we are adding
	*
	* @return boolean
	* @access public
	*/
	function attachQuestion($type_code)
	{
		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$db = MatrixDAL::getDb();

		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		$questions = $this->getQuestions();
		if ($this->attr('next_questionid') > 0) {
			$new_key = $this->attr('next_questionid');
		} else if (empty($questions)) {
			$new_key = 1;
		} else {
			$new_key = max(array_keys($questions)) + 1;
		}

		$questions[$new_key] = Array(
								'question_type_code'	=> $type_code,
								'attributes'			=> Array('name' => 'Question '.(count($questions)+1)),
							   );

		$this->setAttrValue('questions', $questions);
		$this->setAttrValue('next_questionid', $new_key + 1);

		$sort_order = $this->attr('sort_order');

		if (empty($sort_order)) {
			$new_index = 0;
		} else {
			$new_index = max(array_keys($sort_order))+1;
		}
		$sort_order[$new_index] = $this->id.':q'.$new_key;
		$this->setAttrValue('sort_order', $sort_order);

		if (!$this->saveAttributes()) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return FALSE;
		}

		// acquire the lock on the new question
		$GLOBALS['SQ_SYSTEM']->am->acquireLock($this->id.':q'.$new_key, 'all', $this->id, TRUE);
		$GLOBALS['SQ_SYSTEM']->am->updateLock($this->id, 'all');

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		return TRUE;

	}//end attachQuestion()


	/**
	* delete a question to this form
	*
	* Returns boolean FALSE if a database problem occurs or if question already
	* attached, or TRUE if sucessful
	*
	* @param object	&$question	the Form_Question object to delete
	*
	* @return boolean
	* @access public
	*/
	function deleteQuestion(&$question)
	{
		if ($question->_questionid == 0) return FALSE;

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
		$questions = $this->getQuestions();

		unset($questions[$question->_questionid]);

		$this->setAttrValue('questions', $questions);

		$sort_order = $this->attr('sort_order');

		// remove it from the sort order - it does not matter that there are gaps
		$old_index = array_search($question->id, $sort_order);
		unset($sort_order[$old_index]);

		// unset the question
		unset($question);

		$this->setAttrValue('sort_order', $sort_order);

		if (!$this->saveAttributes()) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return FALSE;
		}
		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		return TRUE;

	}//end deleteQuestion()


	/**
	* returns the number of sections in this form
	*
	* @return int
	* @access public
	*/
	function getQuestionCount()
	{
		$questions = $this->getQuestions();
		return count($questions);

	}//end getQuestionCount()


	/**
	* get a specific question, and return the question's link
	*
	* @param string	$questionid	The question's (shadow?) ID
	*
	* @return array
	* @access public
	*/
	function getQuestionByID($questionid)
	{
		$questions = $this->getQuestions();
		return (isset($questions[$questionid])) ? $questions[$questionid] : Array();

	}//end getQuestionByID()


	/**
	* get specific question by order, and return the question's link
	*
	* @param int	$orderid	The question's sort order
	*
	* @return array
	* @access public
	*/
	function &getQuestionByOrder($orderid)
	{
		$sort_order = $this->attr('sort_order');
		$question = $GLOBALS['SQ_SYSTEM']->am->getAsset($sort_order[$orderid]);
		return $question;

	}//end getQuestionByOrder()


//--        SECTION MANAGEMENT        --//


	/**
	* returns immediate sections in this form in order of 'sort_order'
	*
	* Structure of return array is Array(object Form_Section).
	*
	* @return array
	* @access public
	*/
	function &getSections()
	{
		$am = $GLOBALS['SQ_SYSTEM']->am;
		$section_links = $this->getSectionLinks();
		$sections = Array();

		foreach ($section_links as $link) {
			$sections[$link['sort_order']] = $am->getAsset($link['minorid'], $link['minor_type_code']);
		}
		return $sections;

	}//end getSections()


	/**
	* Returns all sections regardless of depth
	*
	* Structure of return array is Array(object Form_Section).
	*
	* @return array
	* @access public
	*/
	function &getAllSections()
	{
		$am = $GLOBALS['SQ_SYSTEM']->am;
		$section_links = $this->getSectionLinks();
		$sections = Array();

		while (!empty($section_links)) {
			$link = array_shift($section_links);
			$sections[] = $am->getAsset($link['minorid'], $link['minor_type_code']);

			// get subsection links and add them onto the front
			$sub_section_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($link['minorid'], SQ_SC_LINK_ALL, 'form_section', FALSE);
			$section_links = array_merge($sub_section_links, $section_links);
		}
		return $sections;

	}//end getSections()


	/**
	* returns the number of sections in this form
	*
	* @return int
	* @access public
	*/
	function getSectionCount()
	{
		return $GLOBALS['SQ_SYSTEM']->am->countLinks($this->id, 'major', SQ_LINK_TYPE_2, 'form_section', TRUE);

	}//end getSectionCount()


	/**
	* Get the question links attached to this form (and only to this form).
	*
	* Returns an array of (question ID => question detail).
	*
	* @return array
	* @access public
	*/
	function &getSectionLinks()
	{
		// This is what one would find in Asset_Manager::&getAsset() except we can't call that,
		// lest we spin off into an infinite loop involving OUR &getAsset().
		$value = Array(
					'link_value' 	=> Array(''),
					'equal' 		=> TRUE,
				 );
		$links_query = $GLOBALS['SQ_SYSTEM']->am->generateGetLinksQuery($this->id, SQ_LINK_TYPE_2, 'form_section', TRUE, 'major', $value, '1', '0');
		if (empty($links_query)) return Array();
		$links_query['sql_array']['select'] .= ', l.majorid';		// we need the major id too

		try {
			$sql = implode(' ', $links_query['sql_array']);
			$query = MatrixDAL::preparePdoQuery($sql);

			foreach ($links_query['bind_vars'] as $bind_var => $bind_value) {
				MatrixDAL::bindValueToPdo($query, $bind_var, $bind_value);
			}

			$result = MatrixDAL::executePdoAssoc($query);
		} catch (Exception $e) {
			throw new Exception('Unable to get section links attached to form: '.$this->id.' due to database error: '.$e->getMessage());
		}
		return $result;

	}//end getSectionLinks()


//--        BODYCOPIES        --//


	/**
	* Print the body of this asset
	*
	* @return void
	* @access public
	**/
	function printBody()
	{
		// Pick up the current submission, if any.
		$submission = $this->getSubmissionAsset();
		if ($submission === TRUE) {
			// Public User is accessing a previously saved submission,
			// hence will have to go through the password verification form first
			return;
		}

		// If Public User is allowed to save the incomplete submission
		$public_save = $this->attr('public_incomplete_submission');

		// If user is public and the submission is unsaved
		if (is_null($submission) && $GLOBALS['SQ_SYSTEM']->userPublic()) {
			$this->restoreFromSubmissionCookie();
			$submission_var =& $_SESSION['SQ_FORM_'.$this->id.'_SUBMISSION'];
		}

		if (isset($submission)) {
			$current_page = array_get_index($_POST, 'SQ_FORM_'.$this->id.'_PAGE', $submission->attr('current_page'));
			$latest_page = $submission->attr('latest_page');
		} else if (isset($submission_var)) {
			$current_page = array_get_index($_POST, 'SQ_FORM_'.$this->id.'_PAGE', array_get_index($submission_var, 'current_page', NULL));
			$latest_page = array_get_index($submission_var, 'latest_page', NULL);
		} else {
			$current_page = $latest_page = NULL;
		}

		if ($current_page) {
			$this->current_page_questions = array_keys($this->getQuestionAssetsOnPage($current_page));
		}

		$prefix         = $this->getPrefix();
		$total_pages    = $this->getTotalPages();

		$action = NULL;
		$possible_actions = Array(
								'submit',
								'previous_page',
								'reset',
								'save',
								'save_and_exit',
							);

		foreach ($possible_actions as $possible_action) {
			if (isset($_POST[$prefix.'_'.$possible_action])) {
				$action = $possible_action;
				break;
			}
		}

		$submission_limit = $this->attr('submission_limit');
		// see if the submission limit is set and has reached
		// if so display the appropiate bodycopy
		if (is_null($action) && $submission_limit != 0 && $submission_limit <= $this->getSubmissionCount()) {
			$this->printSubmissionLimitReachedBodycopy();
			return;
		}

		$jump_to_page   = array_get_index($_GET, 'SQ_FORM_'.$this->id.'_PAGE', NULL);
		$finished       = FALSE;

		// Process the current page, if any - process if:
		// - we are moving to the next or previous page (save submission), OR
		// - we are jumping to an arbitrary page (do NOT save submission).
		if (!is_null($current_page) || !is_null($jump_to_page) || (!is_null($action) && ($action !== 'reset'))) {
			// Are we jumping to another page? If not, then we do NOT save the
			// submission at this point.
			if (!is_null($jump_to_page)) {
				$current_page = $jump_to_page;
			} else {
			    
				// Only validate the page responses if the "Next/Submit" button is pressed.
				// In other words, allow gunky data to be stored because we
				// aren't actually progressing in the form; we will get sto it
				// when we go forward from this page (whenever that happens).
				$validate_page = ($action === 'submit') ? TRUE : FALSE;
				// Mute validation errors if there is no action, such as a public user refreshs file upload question form
				$mute_errors = (empty($action)) ? TRUE : FALSE;

				if (is_null($current_page)) $current_page = 1;
				$success = $this->processPage($current_page, $validate_page, $mute_errors);

				// Run the step submission actions for current page, if any
				$success = $success && (!$validate_page || $this->performFormSectionActions($current_page)) && empty($this->submission_errors);

				if ($success) {

					// If the submission has not yet been created, create the submission
					// in the "Incomplete Submissions" folder - but if there was only
					// one page, we can fudge it

					if (!isset($submission_var) && !$submission) {
						$create_in_completed = ($total_pages == 1) ? TRUE : FALSE;

						// Get an incomplete submissions, and now we know
						// this is page one
						//if (!$create_in_completed) {
							$submission = $this->createSubmission($create_in_completed);
							$this->submission_asset = $submission;
						//}
					}

					// If we have a submission, then, update it
					if (isset($submission)) {
						$this->updateSubmission($submission);
					} else if (isset($submission_var)) {
						$this->updateSubmissionCookie();
					}

					// Are we there yet? Assuming that we are moving forward
					// on this page, and not backwards or same page...
					if (($current_page == $total_pages) && ($action === 'submit')) {
						$finished = TRUE;
					} else {
						if ($action === 'previous_page') {
							$current_page--;
						} else if ($action === 'submit') {
							$current_page++;
						}

						$current_page = max($current_page, 1);
					}
				}
			}
		} else {
			// just change it to first page as we haven't been to any page yet
			$latest_page = $current_page = 1;
		}

		// Save the incomplete submission for the Public User
		// For the logged user, submission is saved automatically
		if ($public_save && !isset($submission) && !$finished && isset($submission_var) && ($action == 'save_and_exit' || $action == 'save')) {
			$submission = $this->createSubmission($total_pages == 1);
			$this->submission_asset = $submission;
			$this->updateSubmission($submission, TRUE);

			// Send the email notification to the Public User
			$this->sendPublicUserSubmissionInfoEmail();

			// Get rid of public user submission data in the session, we wont be needing it anymore
			unset($_SESSION['SQ_FORM_'.$this->id.'_SUBMISSION']);
		}

		// We have to do this here, because we cannot break the old behaviour
		// where even the summary/thank you page is surrounded by form tags

		if ($action === 'save_and_exit') {
			echo $this->getExitPage();
		} else {
			if (!$finished) {
				// If the submission hasn't been created yet, don't bother storing
				// current page or anything like
				if (isset($submission)) {
					$submission->setAttrValue('current_page', $current_page);
					if ($current_page > $latest_page) {
						$submission->setAttrValue('latest_page', $current_page);
					}
					$submission->saveAttributes(FALSE, $this->attr('log_submissions'));
				} else if (isset($submission_var)) {
					$_SESSION['SQ_FORM_'.$this->id.'_SUBMISSION']['current_page'] = $current_page;
					if ($current_page > $latest_page) {
						$_SESSION['SQ_FORM_'.$this->id.'_SUBMISSION']['latest_page'] = $current_page;
					}
				}
			}

			if (!$finished) {
				echo $this->getFormHeader();
				// Print the current page or the Confirmation Page if we're confirming things
				$total_pages = $this->getTotalPages();

				if (($current_page == $total_pages) && $this->attr('use_confirmation_page')) {
					echo $this->getConfirmationPageContents();
				}
				else {
					$this->printPageContentsBodycopy();
				}
				echo $this->getFormFooter();
			} else {
				$ok = $this->completeSubmission();
				if ($ok) {
					// On-submit actions went through okay, so we can finish
					// the form
					$this->finishForm();
				} else {
					echo $this->getFormHeader();
					// We had problems, so re-print the last page
					$this->printPageContentsBodycopy();
					echo $this->getFormFooter();
				}
			}

		}

		if ($finished) {
			$this->_finaliseStepsActions();
		}

	}//end printBody()


	/**
	* Returns the header of the form, ie. the start form tags and anything else
	* needed by them
	*
	* @return string
	*/
	public function getFormHeader()
	{
		$form_prefix = $this->getPrefix();
		$form_action = $this->getFormAction();

		$header = '';
		if ($this->attr('use_client_side')) {
			$on_submit = 'onsubmit="return beforeSubmit_'.$form_prefix.'(this);"';
			$header .= '<form id="'.$form_prefix.'" enctype="multipart/form-data" action="'.$form_action.'" method="post" '.$on_submit.'>';
			$header .= $this->getClientSideFunction();
		} else {
			$header .= '<form id="'.$form_prefix.'" enctype="multipart/form-data" action="'.$form_action.'" method="post">';
		}

		$current_submission = $this->getSubmissionAsset();

		$submission_var = array_get_index($_SESSION, 'SQ_FORM_'.$this->id.'_SUBMISSION', NULL);

		if ($current_submission) {
			ob_start();
				hidden_field('SQ_FORM_'.$this->id.'_SUBMISSION', $current_submission->id);
				hidden_field('SQ_FORM_'.$this->id.'_PAGE', $current_submission->attr('current_page'));
			$header .= ob_get_clean();
		} else if ($submission_var) {
			ob_start();
				hidden_field('SQ_FORM_'.$this->id.'_PAGE', array_get_index($submission_var, 'current_page', 1));
			$header .= ob_get_clean();
		} else {
			ob_start();
				hidden_field('SQ_FORM_'.$this->id.'_PAGE', 1);
			$header .= ob_get_clean();
		}

		return $header;

	}//end getFormHeader()


	/**
	* Returns the action this form should be using
	*
	* @return string
	*/
	public function getFormAction()
	{
		$form_action = current_url();

		// Add the query string as well if it exists, so asset references with ?a=###
		// submit back to themselves.
		// BUT, remove internal page or submission override GET variables, otherwise
		// we'll be stuck at the one page once a GET var comes in
		if (!empty($_SERVER['QUERY_STRING'])) {
			preg_match_all('/([^=&]*)\=([^&]*)/', $_SERVER['QUERY_STRING'], $matches);
			$matches = array_combine($matches[1], $matches[2]);

			foreach ($matches as $key => $value) {
				if (($key === 'SQ_FORM_'.$this->id.'_PAGE') || ($key === 'SQ_FORM_'.$this->id.'_SUBMISSION') || ($key === 'SQ_FORM_'.$this->id.'_ACCESSID')) {
					unset($matches[$key]);
				} else {
					// URLdecode the match because make_raw_post_data() will re-encode it
					$matches[$key] = urldecode($matches[$key]);
				}
			}

			$form_action .= '?'.htmlspecialchars(make_raw_post_data($matches), ENT_NOQUOTES, SQ_CONF_DEFAULT_CHARACTER_SET);
		}

		return $form_action;

	}//end getFormAction()


	/**
	* Print a single page of a multiple page form
	*
	* Page number is 1-indexed. If there are unattached questions, this will be
	* page 1, then the first section is page 2. If no unattached questions,
	* the first section is page 1.
	*
	* @param int	$page_number	The current page number
	*
	* @return string
	*/
	public function getPageContents($page_number)
	{
		$page_number = max($page_number, 1);
		ob_start();

		// Firstly, find out whether we should actually be displaying the
		// confirmation page: if it is enabled and we are supposedly on the
		// final page, it MUST be the confirmation page
		$total_pages = $this->getTotalPages();

		if (($page_number == $total_pages) && $this->attr('use_confirmation_page')) {
			echo $this->getConfirmationPageContents();
		} else {
			// We are on a "normal" form page
			if ($this->attr('multi_page')) {
				// do we have any unattached questions?
				$questions = $this->getQuestions();

				// Get top-level sections and re-base their indices
				$sections = $this->getSectionLinks();

				// If we have unattached questions, then reduce the page number
				// internally - so that section 1 will be on page 2
				if (!empty($questions)) {
					$page_number--;
				}

				if ($page_number == 0) {
					// Printing unattached questions (if any)
					$bodycopy = $this->getBodycopy('Unattached Questions Format');

					// check to see if the unattached bodycopy exists and has any content
					if ($this->attr('use_bodycopy_unattached') && $bodycopy && !$bodycopy->isEmpty()) {
						$keyword_list = $bodycopy->getKeywords();

						if (!empty($keyword_list)) {
							foreach ($keyword_list as $keyword) {
								// Question and section-related keywords are found here
								$replacement = $this->getDisplayKeywordReplacement($keyword);

								if (is_null($replacement)) {
									// If not picked up by first function, this will pick up
									// replacements placed in separate functions - CAPTCHA,
									// errors, for instance
									$replacement = $this->getKeywordReplacement($keyword);
								}

								if (!is_null($replacement)) {
									// We have something from one of the above
									$replacements[$keyword] = $replacement;
								}
							}
						}

						$bodycopy->setKeywordReplacements($replacements);
						$bodycopy->printBody();

					} else {
						// Grab the default "unattached questions" content file (at the moment)
						require_once $this->data_path.'/content_file_questions.php';
					}
				} else {
					// Printing a section - sections are zero based after they were
					// re-based by array_values
					$section_link = reset(array_slice($sections, $page_number - 1, 1));
					$section = $GLOBALS['SQ_SYSTEM']->am->getAsset($section_link['minorid']);
					$section->printBody($this);
				}
			} else {
				$this->printPageContentsBodycopy();
			}
		}

		$contents = ob_get_clean();
		return $contents;

	}//end printPage()


	/**
	* Returns the form footer and anything else needed by it
	*
	* @return string
	*/
	public function getFormFooter()
	{
		return '</form>';

	}//end getFormFooter()


	/**
	* Print 'Page Contents' Bodycopy context
	*
	* @return void
	* @access public
	*/
	function printPageContentsBodycopy()
	{
		$this->printForm();

	}//end printPageContentsBodycopy()


	/**
	* Print 'Thank You' Bodycopy context
	*
	* @return void
	* @access public
	*/
	function printThankYouBodycopy()
	{
		// Make sure that the thank you bodycopy is always able to access the
		// newly created submission in replicated database instances, by tying
		// it all to db2
		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$this->finishForm();
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

	}//end printThankYouBodycopy()


	/**
	* Prints the form to screen
	*
	* Parameter $replacements is DEPRECATED; consider placing additional
	* replacements in a subclassed getDisplayKeywordReplacements(), or their
	* own function, instead.
	*
	* @param array	$replacements	a list of additional replacements that
	*								subclasses can use to insert additional
	*								keywords
	*
	* @return void
	* @access public
	*/
	function printForm($replacements=Array())
	{
		$bodycopy = $this->getBodycopy('Page Contents');

		// check to see if the bodycopy attached to this form has any content
		if ($this->attr('use_bodycopy_main') && $bodycopy && !$bodycopy->isEmpty()) {
			$keyword_list = $bodycopy->getKeywords();

			if (!empty($keyword_list)) {
				foreach ($keyword_list as $keyword) {
					// Question and section-related keywords are found here
					$replacement = $this->getDisplayKeywordReplacement($keyword);

					if (is_null($replacement)) {
						// If not picked up by first function, this will pick up
						// replacements placed in separate functions - CAPTCHA,
						// errors, for instance
						$replacement = $this->getKeywordReplacement($keyword);
					}

					if (!is_null($replacement)) {
						// We have something from one of the above
						$replacements[$keyword] = $replacement;
					}
				}
			}

			$bodycopy->setKeywordReplacements($replacements);
			$bodycopy->printBody();

		} else {
			$multi_page = $this->attr('multi_page');
			echo $this->printDefaultPageContents($multi_page);

		}//end else

	}//end printForm()


	/**
	* Prints the default page contents
	*
	* @param boolean	$multi_page	Whether to print single section in multiple mode or all sections
	*											of this form. If multi-page is turned on,
	*											this should be FALSE.
	*
	* @return string
	*/
	public function printDefaultPageContents($multi_page=FALSE)
	{
		$current_page = $this->getCurrentPageNumber();

		$error = !empty($this->submission_errors);

		if($this->attr('use_accessible_format')) {
		    $is_required_field = FALSE;
		    $questions = $this->getAllQuestions();
		    foreach ($questions as $question) {
			if(isset($question['attributes']['is_required']) && $question['attributes']['is_required']) {
			    $is_required_field = TRUE;
			}
		    }
		    $require_note = $this->attr('required_field_note');
		    if(empty($require_note))
			$is_required_field =  FALSE;
		    
		    // new semantic format
		    ob_start();
		    // if there is required field, display the required field note
		    if($is_required_field) {
		    ?>
		<div class="sq-form-required-field-note"><?php echo $require_note; ?></div>
		    <?php
		    }?>
		    %<?php echo $multi_page ? 'current_page' : 'form' ?>_contents%
		    <div class="sq-form-control">
		    <?php
				    if ($this->attr('require_captcha') && ($current_page == $this->getTotalPages())) {
					    ?>
					    <div class="sq-form-captcha">
					    %form_captcha%
					    </div>
					    <?php
				    }
				    else if($this->attr('use_mollom')) {
					  ?>
					    <div class="sq-form-captcha">
					    %form_mollom_captcha%
					    </div>
					    <?php
				    }
				    ?>
				    %submit_button%
				    <?php
				    if ($error) {
					    ?>
					    <div class="sq-form-error">%form_errors%</div>
					    <?php
				    }
				    ?>
		  </div>
		  <?php  
		    $content = ob_get_clean();
		}
		else {
		    // old tabular format
		    $form_width = $this->attr('form_width');
		    if (!empty($form_width))     $form_width = ' width="'.$form_width.'"';
		 
		    $question_width = $this->attr('question_col_width');
		    if (!empty($question_width))     $question_width = ' width="'.$question_width.'"';
		    
		    $answer_width = $this->attr('answer_col_width');
		    if (!empty($answer_width))     $answer_width = ' width="'.$answer_width.'"';

		    ob_start();
		    ?>
		    %<?php echo $multi_page ? 'current_page' : 'form' ?>_contents%<table <?php echo $form_width; ?>>
			    <tr>
			    <td <?php echo $question_width; ?>>&nbsp;</td>
			    <td <?php echo $answer_width; ?>>
				    <?php
				    if ($this->attr('require_captcha') && ($current_page == $this->getTotalPages())) {
					    ?>
					    %form_captcha%
					    <?php
				    }
				    else if($this->attr('use_mollom')) {
					  ?>
					    <div class="sq-form-captcha">
					    %form_mollom_captcha%
					    </div>
					    <?php
				    }
				    ?>
				    %submit_button%
				    %reset_button%
			    </td>
			    </tr>
			    <?php
			    if ($error) {
				    ?>
				    <tr><td colspan="2">%form_errors%</td></tr>
				    <?php
			    }
			    ?>
			    </table>
		    <?php
		    $content = ob_get_clean();
		}

		require_once SQ_FUDGE_PATH.'/general/text.inc';

		$keyword_list = extract_keywords($content);
		if (!empty($keyword_list)) {
			foreach ($keyword_list as $keyword) {
				if (isset($replacements[$keyword])) continue;
				$replacements[$keyword] = $this->getKeywordReplacement($keyword);
			}

			replace_keywords($content, $replacements);
		}
		return $content;

	}//end printDefaultPageContents()


	/**
	* Gets the contents of a "resume submission" page
	*
	*/
	public function getResumeSubmissionPageContents()
	{


	}//end getResumeSubmissionPageContents()


	/**
	* Prints the standard form i.e not a wizard
	*
	* @return boolean
	* @access public
	*/
	function printStandard()
	{
		$prefix = $this->getPrefix();

		$form_processed = FALSE;

		// check to see if the form has been submitted or not
		if (isset($_POST[$prefix.'_submit'])) {
			$form_processed = $this->processForm();
		}

		if ($form_processed) {
			$this->printThankYouBodycopy();
		} else {
			$this->printPageContentsBodycopy();
		}

		return TRUE;

	}//end printStandard()


	/**
	* Create the "Exit Page", which will be displayed when "Save and Exit" is
	* selected on a form
	*
	* If the Exit Page Contents bodycopy is not customised, then... ???
	*
	* @return string
	*/
	public function getExitPage()
	{
		$contents = '';

			if ($this->attr('use_bodycopy_exit')) {
			$bodycopy = $this->getBodycopy('Exit Page Contents');

			if ($bodycopy) {
				ob_start();

				$keyword_list = $bodycopy->getKeywords();
				$replacements = Array();

				// Get the keyword replacement for "incomplete submissions"
				$keywords_str = !empty($keyword_list) ? ('%'.implode('%%', $keyword_list).'%') : '';
				$replacements = $this->getIncompleteSubmissionMessageKeywordsReplacement($keywords_str);

				foreach ($keyword_list as $keyword) {
					// Allow all other display keywords, but explicitly prohibit
					// HTML field keywords
					if (strpos($keyword, 'question_field_') !== 0) {
						$keyword_value = $this->getDisplayKeywordReplacement($keyword);
						if (!empty($keyword_value)) {
							$replacements[$keyword] = $keyword_value;
						}
					}
				}

				$bodycopy->setKeywordReplacements($replacements);
				$bodycopy->printBody();

				$contents = ob_get_clean();
			}
		}

		return $contents;

	}//end getExitPage()


	/**
	* create a bodycopy and attach to the Bodycopies folder of the form
	*
	* The $name attribute will also be used for the Link Value of the link between
	* the form's Bodycopies folder and the new bodycopy. The name will be lowercased,
	* spaces replaced with underscores, and assigned to the link value. So if 'Thank
	* You' is the name of the bodycopy (for instance), it will be linked to the
	* Bodycopies folder with the link value 'thank_you'.
	* Returns boolean FALSE if the bodycopy already exists or if the creation
	* failed for some reason, or TRUE if it was successfully created
	*
	* @param string	$name	the name of the bodycopy to create
	*
	* @return boolean
	* @access public
	*/
	function createBodycopy($name, $content=Array())
	{
		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$db = MatrixDAL::getDb();

		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		// search for a bodycopies folder and create one if there isn't one there
		$folder_link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_TYPE_2, 'folder', TRUE, 'bodycopies_folder');
		if (empty($folder_link)) {

			// create a folder for the bodycopies
			$GLOBALS['SQ_SYSTEM']->am->includeAsset('folder');
			$folder = new Folder();
			$folder->setAttrValue('name', 'Bodycopies');

			$folder_link = Array('asset' => &$this, 'link_type' => SQ_LINK_TYPE_2, 'value' => 'bodycopies_folder', 'is_dependant' => 1, 'is_exclusive' => 1);

			if (!$folder->create($folder_link)) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
				return FALSE;
			}
		} else {
			// folder already exists, get the reference
			$folder = $GLOBALS['SQ_SYSTEM']->am->getAsset($folder_link['minorid'], $folder_link['minor_type_code']);
		}
		if (is_null($folder)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return FALSE;
		}

		// and see whether this bodycopy already exists
		$bodycopy_link = $GLOBALS['SQ_SYSTEM']->am->getLink($folder->id, SQ_LINK_TYPE_2, 'bodycopy', TRUE, $name);
		if (!empty($bodycopy_link)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return TRUE;
		}

		// bodycopy doesn't exist, so create it
		// create a link to the folder so we can put the bodycopies in it
		$copy_link = Array('asset' => &$folder, 'link_type' => SQ_LINK_TYPE_2, 'is_dependant' => 1, 'is_exclusive' => 1, 'value' => $name);

		// create the thing
		$GLOBALS['SQ_SYSTEM']->am->includeAsset('bodycopy');
		$bodycopy = new Bodycopy();
		$bodycopy->setAttrValue('name', $name);

		// create an appropriate value for this link
		// to reflect that of the name
		$name = strtolower(str_replace(' ', '_', $name));

		if (!$bodycopy->create($copy_link, $content)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return FALSE;
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		return TRUE;

	}//end createBodycopy()


	/**
	* returns a reference to a bodycopy based of the name of the bodycopy
	*
	* @param string	$name	the name of the bodycopy
	*
	* @return object
	* @access public
	*/
	function &getBodycopy($name='', $link_type=SQ_LINK_TYPE_2)
	{
		$null = NULL;

		if (!$name) return $null;
		$am = $GLOBALS['SQ_SYSTEM']->am;

		$folder_link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_TYPE_2, 'folder', TRUE, 'bodycopies_folder');
		if (empty($folder_link)) return $null;

		$folder = $am->getAsset($folder_link['minorid'], $folder_link['minor_type_code']);
		if (is_null($folder)) return $null;

		$bodycopy_link = $GLOBALS['SQ_SYSTEM']->am->getLink($folder->id, $link_type, 'bodycopy', TRUE, $name);
		if (empty($bodycopy_link)) return $null;

		$bodycopy = $am->getAsset($bodycopy_link['minorid'], $bodycopy_link['minor_type_code']);

		return $bodycopy;

	}//end getBodycopy()


	/**
	* includes the content file for this form
	*
	* Options available for $type:
	* - "Standard":  include the whole form, including sections. Useful for single-page form.
	* - "Questions": only include unattached questions. Useful for a multi-page form.
	*
	* @param string	$type	the type of content file to generate
	*
	* @return boolean
	* @access public
	*/
	function includeContentFile($type='Standard')
	{
		$edit_fns = $this->getEditFns();

		switch($type) {
			case 'Standard':
				if (!file_exists($this->data_path.'/content_file.php')) {
					if (!$edit_fns->generateStandardContentFile($this)) {
						return FALSE;
					}
				}
				include($this->data_path.'/content_file.php');
			break;

			case 'Questions':
				if (!file_exists($this->data_path.'/content_file_questions.php')) {
					if (!$edit_fns->generateQuestionsContentFile($this)) {
						return FALSE;
					}
				}
				include($this->data_path.'/content_file_questions.php');

			break;
		}

		return TRUE;

	}//end includeContentFile()


	/**
	* Returns the total number of pages in this form.
	*
	* If multi-page form is DISABLED, then the whole form is one page.
	* If ENABLED, then each top-level section becomes a page, and any unattached
	* questions share one extra page, if any.
	*
	* In either case, if the confirmation page is enabled, add one page.
	*
	* @return int
	*/
	public function getTotalPages()
	{
		if ($this->attr('multi_page')) {
			$pages = $this->getSectionCount() + ($this->getQuestionCount() > 0 ? 1 : 0);
		} else {
			$pages = 1;
		}

		// if confirmation page exists, add 1 page
		if ($this->attr('use_confirmation_page')) {
			$pages++;
		}

		return $pages;

	}//end getTotalPages()


//--        CONFIRMATION PAGE FUNCTIONS        --//


	/**
	* Get the Confirmation Page, wherever it comes from
	*
	* @return string
	*/
	public function getConfirmationPageContents()
	{
		if ($this->attr('use_bodycopy_confirmation')) {
			$bodycopy = $this->getBodycopy('Confirmation Page Contents');
			$keyword_list = $bodycopy->getKeywords();
		} else {
			$contents = $this->getDefaultConfirmationPageContents();
			$keyword_list = extract_keywords($contents);
		}

		// Replace keywords
		$replacements = Array();
		if (!empty($keyword_list)) {
			foreach ($keyword_list as $keyword) {
				$lookup_keyword = $keyword;
				// If form_info or form_summary, we need to make sure the
				// HTML version gets printed
				if (($keyword === 'form_info') || ($keyword == 'form_summary')) {
					$lookup_keyword = $keyword.'_html';
				}
				$replacements[$keyword] = $this->_getThankYouKeywordReplacement($lookup_keyword);
			}
		}

		if ($this->attr('use_bodycopy_confirmation')) {
			ob_start();
				$bodycopy->setKeywordReplacements($replacements);
				$bodycopy->printBody();
			$contents = ob_get_clean();
		} else {
			replace_keywords($contents, $replacements);
		}

		return $contents;

	}//end getConfirmationPageContents()


	/**
	* Get the default contents of the Confirmation Page
	*
	* @return string
	*/
	public function getDefaultConfirmationPageContents()
	{
		ob_start();
		if($this->useAccessibleFormat()) {
		?>
		%form_summary_semantic%

		<?php
		}
		else {
		?>
		%form_summary_html%

		<?php
		}
		if ($this->attr('require_captcha')) {
			?>
			<p class="sq-form-captcha">%form_captcha%</p>
			<?php
		}
		?>
		%submit_button% %previous_page_button%
		<div class="sq-form-error">%form_errors%</div>
		<?php
		$contents = ob_get_clean();

		return $contents;

	}//end getDefaultConfirmationPageContents()


//--        FORM PROCESSING        --//


	/**
	* regenerates the content file for this form
	*
	* @return boolean
	* @access public
	*/
	function fileRegeneration()
	{
		$edit = $this->getEditFns();
		return $edit->generateStandardContentFile($this);

	}//end fileRegeneration()


	/**
	* Processes the specified page
	*
	* @param int		$page_number	The page number to be processed
	* @param boolean	$validate_page	If TRUE, the function will return FALSE
	*									if the question values are invalid or
	*									the CAPTCHA is incorrectly entered. This
	*									is usually only set TRUE when moving
	*									forward in the form
	* @param boolean	$mute_errors	Mute validation error
	*
	* @return boolean
	*/
	public function processPage($page_number, $validate_page=TRUE, $mute_errors=FALSE)
	{
		$ok = TRUE;
		// If the form has only one page,  it doens't have incomplete submission asset nor session to store captcha_status.
		// this var is used to make sure captcha status is processed correctly in this case
		$one_page_captcha_status = FALSE;

		// Has a captcha been completed on this page? Error if it is incorrect,
		// otherwise set the status that we have passed one
		// Only when we're going forward, though.
		$submission = $this->getSubmissionAsset();

		if ($validate_page && isset($_POST['SQ_SYSTEM_SECURITY_KEY_VALUE'])) {
			if (validate_security_key()) {
				$submission = $this->getSubmissionAsset();

				if (!empty($submission)) {
					$submission->setAttrValue('captcha_status', TRUE);
					$submission->saveAttributes(FALSE,  $this->attr('log_submissions'));
				} else if (isset($_SESSION['SQ_FORM_'.$this->id.'_SUBMISSION'])) {
					$submission_var =& $_SESSION['SQ_FORM_'.$this->id.'_SUBMISSION'];
					$submission_var['captcha_status'] = TRUE;
				} else if ($this->getTotalPages() == 1) {
					$one_page_captcha_status = TRUE;
				}

			} else {
				$this->registerErrors('captcha_errors', Array($this->attr('captcha_invalid_error_message')));
				$ok = FALSE;
			}
		}

		// We have got to the last page (and are moving forward, ie. submitting),
		// but we need a captcha to pass and we don't have it yet!
		if (!empty($submission)) {
			$captcha_status = $submission->attr('captcha_status');
		} else if (isset($_SESSION['SQ_FORM_'.$this->id.'_SUBMISSION'])) {
			$submission_var =& $_SESSION['SQ_FORM_'.$this->id.'_SUBMISSION'];
			$captcha_status = array_get_index($submission_var, 'captcha_status', FALSE);
		} else if ($one_page_captcha_status === TRUE) {
				$captcha_status = TRUE;
		} else {
			$captcha_status = FALSE;
		}

		// first thing we need to check is if the number of submission is limited
		// and if so see if the submission limit has been reached while this user
		// was still filling up the form
		$submission_limit = $this->attr('submission_limit');
		if ($submission_limit != 0 && $submission_limit <= $this->getSubmissionCount()){
			$error_text = $this->attr('submission_limit_reached_text');
			$this->registerErrors('submission_errors', Array($error_text));
			$ok = FALSE;
		}

		// only display this error message if captcha error isn't displayed at current page
		if (($page_number == $this->getTotalPages()) && empty($this->captcha_errors)  && $validate_page && !$captcha_status && $this->attr('require_captcha')) {
			$this->registerErrors('captcha_errors', Array($this->attr('captcha_form_invalid_error_message')));
			$ok = FALSE;
		}

		// Questions to be verified by Mollom
		$questions_to_verify = $this->attr('mollom_questions_to_check');
		$mollom_parameters = Array();

		$questions = $this->getQuestionAssetsOnPage($page_number);
		for (reset($questions); $question =& $questions[key($questions)]; next($questions)) {

			// There is no point in populating the question if answer is not posted
			if (isset($_POST['SQ_FORM_'.$this->id.'_PAGE'])) {
				$question->populate($this);
				$content_type = isset($questions_to_verify[$question->id]) ? $questions_to_verify[$question->id] : -1;
				if ($content_type > -1) {
					$mollom_parameters[$content_type] = $question->getValue();
				}
			}

			// Only fail the page if we are moving forward
			// File upload question should always get validated
			$question_type = strtolower($question->type());
			$valid_value = $question->hasValidValue(NULL, FALSE);

		    if (!$valid_value && (($question_type == 'form_question_type_file_upload') || $validate_page)) {
		    	//mute error if there is no action, such as a public user refreshs page when there is a file upload question
		    	if (!$mute_errors) {
			    $this->registerErrors('question_errors', Array($question->id => $question->failed_rules));
			 }
			$ok = FALSE;
		    }

			$this->current_answers[$question->id] = $question->getValue();
			$this->extra_data[$question->id]      = $question->getExtraData();
		}

		// Mollom spam check
		if ($ok && $this->attr('use_mollom') && !empty($mollom_parameters)) {
			$mollom_result = Mollom::verifySubmission($this->id, $mollom_parameters);
			$mollom_error = '';
			switch ($mollom_result) {
				case SQ_MOLLOM_SPAM:
					// Mollom has identified this submission as spam
					$error_text = $this->attr('mollom_spam_submission_text');
					if ($error_text) {
						$mollom_error = $error_text;
					} else {
						$mollom_error = translate('cms_form_mollom_spam_submission_message');
					}
				break;

				case SQ_MOLLOM_UNSURE:
					// Mollom is not sure about the submission
					$error_text = $this->attr('mollom_unsure_submission_text');
					if ($error_text) {
						$mollom_error = $error_text;
					} else {
						// do not display error message if it's not required
						$ok = FALSE;
					}
				break;

				case SQ_MOLLOM_HAM:
					// Everything cool
				break;

				case SQ_MOLLOM_UNKNOWN:
					// No response from Mollom
				break;

				case SQ_MOLLOM_INCORRECT_CAPTCHA:
					$error_text = $this->attr('mollom_captcha_incorrect_text');
					if($error_text) {
						$mollom_error = $error_text;
					} else {
						$mollom_error = translate('cms_form_mollom_invalid_captcha_message');
					}
					break;
			}//end switch

			if (!empty($mollom_error)) {
				$this->registerErrors('mollom_errors', Array($mollom_error));
				$ok = FALSE;
			}

		}//end use Mollom

		return $ok;

	}//end processPage()


	/**
	* re-populates the active submissions of its members
	*
	* @return void
	* @access public
	*/
	function repopulate()
	{
		if (empty($this->current_answers) && empty($this->extra_data)) return;

		$questions = $this->getAllQuestionAssets();
		foreach (array_keys($questions) as $key) {
			if (array_key_exists($key, $this->current_answers)) {
				$question = $questions[$key];
				$question->setValue($this->current_answers[$question->id]);
			}

			if (array_key_exists($key, $this->extra_data)) {
				$question = $questions[$key];
				$extra_data = $this->extra_data[$question->id];
				if (!empty($extra_data)) {
					$question->setExtraData($this->extra_data[$question->id]);
				}
			}
		}

	}//end repopulate()


	/**
	* prints the errors that occured in a form submission
	*
	* @return boolean
	* @access public
	*/
	function printErrors()
	{
		if (!empty($this->submission_errors)) {

			// Get rid of duplicate errors, if any
			$this->submission_errors = array_unique($this->submission_errors);
			echo '<ul>';
			foreach ($this->submission_errors as $error) {
			    echo '<li>';
			    echo $error;
			    echo '</li>';
			}
			echo '</ul>';
		}
		return TRUE;

	}//end printErrors()


	/**
	* returns an answer summary in XML format (suitable for submission logs)
	*
	* @param int	$time	The timestamp of the submission, NULL is to use the current time
	*
	* @return boolean
	* @access public
	*/
	function getXML($time=NULL)
	{
		$submission = $this->getSubmissionAsset();
		$submission_id = $submission->id;
		$submission_ip = $submission->getIP();

		ob_start();
		echo '<submission time="'.str_replace(' ', 'T', ts_iso8601((is_null($time) ? time() : $time))).'"';
		echo ' ip="'.$submission_ip.'"';
		echo ' assetid="'.$submission_id.'"';
		if (!is_null($cur_user = $GLOBALS['SQ_SYSTEM']->user)) {
			echo ' user="'.$cur_user->id.'"';
		}
		echo '>';

		// unattached questions first
		$sort_order = $this->attr('sort_order');

		foreach ($sort_order as $sort_order_index => $assetid) {

			// check to see whether this belongs to us (if it isn't then perhaps a section took care)
			if ($this->id == strtok($assetid, ':')) {
				$question = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
				echo $question->getXML($question->getValue());
			}

		}

		// bug fix #6070 Custom Form XML Export does not work with nested sections
		// get the section's XML recursively
		$this->getSectionXML();
		echo '</submission>';

		$contents = ob_get_contents();
		ob_end_clean();

		return $contents;

	}//end getXML()


	/**
	* returns a summary about the answers in the form submission
	*
	* @param object	$asset the form/section object under which we look to get the related XML
	*
	* @return void
	* @access public
	*/
	function getSectionXML($asset=NULL)
	{
		if (is_null($asset)) $asset = $this;

		// now sections
		$sections = $asset->getSections();
		for (reset($sections); NULL !== ($k = key($sections)); next($sections)) {
			$section =& $sections[$k];
			echo '<section id="'.$section->id.'" name="'.$section->attr('name').'">';

			$sort_order = $section->attr('sort_order');
			foreach ($sort_order as $sort_order_index => $assetid) {
				// check to see whether this belongs to the section
				if ($section->id == strtok($assetid, ':')) {
					$question = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
					echo $question->getXML($question->getValue($assetid));
				}
			}//end foreach
			$this->getSectionXML($section);

			echo '</section>';

		}//end for

	}//end getSectionXML()


	/**
	* returns a summary about the answers in the form submission
	*
	* @param boolean	$html	return html formatted or not
	*
	* @return boolean
	* @access public
	*/
	function printSummary($html=FALSE)
	{
		echo ($html) ? '<table>' : '';

		// unattached questions first
		$sort_order = $this->attr('sort_order');

		foreach ($sort_order as $sort_order_index => $assetid) {

			// check to see whether this belongs to us (if it isn't then perhaps a section took care)
			if ($this->id == strtok($assetid, ':')) {
				$question = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
				echo ($html) ? '<tr><td><ul><li>' : '';
				echo $question->attr('name');
				echo ($html) ? ': </li></ul></td><td>' : ' : ';
				if ($html) {
					echo htmlspecialchars($question->getSummary(), ENT_NOQUOTES, SQ_CONF_DEFAULT_CHARACTER_SET);
				} else {
					echo $question->getSummary();
				}
				echo ($html) ? '</td></tr>' : "\n";
			}

		}

		// now sections
		$sections = $this->getAllSections();
		for (reset($sections); NULL !== ($k = key($sections)); next($sections)) {
			$section =& $sections[$k];
			$section->current_answers =& $this->current_answers;
			$section->extra_data =& $this->extra_data;
			echo ($html) ? '<tr><td colspan="2">' : "\n";
			echo ($html) ? '<strong>'.$section->attr('name').'</strong>' : '** '.$section->attr('name').' **';
			echo ($html) ? '</td></tr>' : "\n\n";

			$sort_order = $section->attr('sort_order');

			foreach ($sort_order as $sort_order_index => $assetid) {

				// check to see whether this belongs to the section
				if ($section->id == strtok($assetid, ':')) {
					$question = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
					echo ($html) ? '<tr><td><ul><li>' : '';
					echo $question->attr('name');
					echo ($html) ? ': </li></ul></td><td>' : ' : ';
					if ($html) {
						echo htmlspecialchars($question->getSummary(), ENT_NOQUOTES, SQ_CONF_DEFAULT_CHARACTER_SET);
					} else {
						echo $question->getSummary();
					}
					echo ($html) ? '</td></tr>' : "\n";
				}

			}
			echo ($html) ? '' : "\n";
		}

		echo ($html) ? '</table>' : '';

	}//end printSummary()


	/**
	*
	*/
	public function printSemanticSummary()
	{
		echo '<ul id="sq-form-summary-semantic-'.$this->id.'" class="sq-form-summary-semantic">'."\n";

		if ($this->getQuestionCount() > 0) {
			echo '<li>'."\n";
			echo '<strong>'.htmlspecialchars($this->attr('unattached_page_name'), ENT_NOQUOTES, SQ_CONF_DEFAULT_CHARACTER_SET).'</strong>'."\n";
			echo '<dl>'."\n";
			foreach ($this->attr('sort_order') as $sort_order => $question_assetid) {

				// Make sure that we own this question
				if ($this->id == strtok($question_assetid, ':')) {
					$question = $GLOBALS['SQ_SYSTEM']->am->getAsset($question_assetid);
					echo '<dt>'.$question->attr('name').'</dt>'."\n";
					echo '<dd>'.htmlspecialchars($question->getSummary(), ENT_NOQUOTES, SQ_CONF_DEFAULT_CHARACTER_SET).'</dd>'."\n";
				}

			}
			echo '</dl>'."\n";
			echo '</li>'."\n";
		}

		// now sections - get the section to print itself out
		$section_links = $this->getSectionLinks();
		foreach ($section_links as $section_link) {
			echo '<li>'."\n";
				$section = $GLOBALS['SQ_SYSTEM']->am->getAsset($section_link['minorid']);
				echo '<strong>'.htmlspecialchars($section->attr('name'), ENT_NOQUOTES, SQ_CONF_DEFAULT_CHARACTER_SET).'</strong>'."\n";
				$section->printSemanticSummary();
			echo '</li>'."\n";
		}

		echo '</ul>';

	}//end printSemanticSummary()


	/**
	* returns information about where the form exists
	*
	* @return array
	* @access public
	*/
	function getFormInfo()
	{
		$info = Array();
		$am = $GLOBALS['SQ_SYSTEM']->am;

		$lineage = $am->getLineageFromURL();
		foreach ($lineage as $asset) {
			$info[$asset['type_code'].'_name'] = $asset['name'];
			$info[$asset['type_code'].'_id']   = $asset['assetid'];
		}
		$info['url'] = current_url(TRUE, TRUE);

		if (!is_null($this->submission_asset)) {
			$info['submission_id'] = $this->submission_asset->id;
		}

		return $info;

	}//end getFormInfo()


//--        CLIENT-SIDE VALIDATION        --//


	/**
	* writes JavaScript code for client side validation
	*
	* This function assumes the skeleton of a JS validation function has been created around this code
	* with a form variable named 'form' passed (this is done when generating the content file).
	*
	* @return string
	* @access public
	*/
	function generateJSCode()
	{
		if (!$this->attr('use_client_side')) return '';

		$keywords = Array();
		$use_bc = ($this->attr('use_bodycopy_main') == 1) ? TRUE : FALSE;

		if ($use_bc) {
			// Bodycopy is being used, so lets get its keywords
			$bc = $this->getBodycopy('Page Contents');
			$keywords = $bc->getKeywords();

			// Form contents is printed, which is just like not using a bodycopy(in this context)
			// So we will print all questions' js code.
			if (in_array('form_contents', $keywords) || in_array('current_page_contents', $keywords)) {
				$use_bc = FALSE;
			} else {
				// Check if we have section keywords in bodycopy
				$section_keywords = Array();
				foreach($keywords as $keyword) {
					if (strpos($keyword, 'section_contents_') !== FALSE) {
						$section_id = substr($keyword, strlen('section_contents_'));
						if ($GLOBALS['SQ_SYSTEM']->am->getAssetInfo($section_id, Array('form_section'))) {
							$section = $GLOBALS['SQ_SYSTEM']->am->getAsset($section_id);
							$s_questions = array_keys($section->getQuestions());
							foreach($s_questions as $q_id) {
								$q_keyword = 'question_field_'.$section_id.'_q'.$q_id;
								if (!in_array($q_keyword, $keywords)) {
									$section_keywords[] = $q_keyword;
								}
							}//end foreach
							$GLOBALS['SQ_SYSTEM']->am->forgetAsset($section);
						}//end if
					}//end if
				}//end foreach

				// Merge the questions (keywords) contained in form section
				$keywords = array_merge($keywords, $section_keywords);
			}//end else
		}

		ob_start();
		?>
			submission_errors = new Array();
		<?php
		$code = ob_get_contents();
		ob_end_clean();

		if (!in_array('form_contents', $keywords) && !$use_bc && $this->attr('multi_page')) {
			$page_num  = $this->getCurrentPageNumber();
			$questions = $this->getQuestionAssetsOnPage($page_num);
		} else {
			$questions = $this->getAllQuestionAssets();
		}

		foreach (array_keys($questions) as $key) {
			$question = $GLOBALS['SQ_SYSTEM']->am->getAsset($key);
			$q_code = '';
			if ($use_bc) {
				$keyword_search = 'question_field_'.str_replace(':', '_', $question->id);
				if (in_array($keyword_search, $keywords)) {
					// keyword is present in the bodycopy, we can safely print the js code.
					$q_code = $question->generateJSCode();
				}
			} else {
				$q_code = $question->generateJSCode();
			}
			$code .= $q_code.(empty($q_code) ? '' : "\n");
		}

		return $code;

	}//end generateJSCode()


	/**
	* Builds a JavaScript fn that can be called to initiate client-side validation
	*
	* @return string
	* @access public
	*/
	function getClientSideFunction()
	{
		$code = $this->generateJSCode();
		if (empty($code)) return '';

		ob_start();
			?>
			<script type="text/javascript">
			function beforeSubmit_<?php echo $this->getPrefix() ?>(form)
			{
				var submission_errors = new Array();
				i = 0;

				<?php echo $code; ?>

				if (submission_errors.length > 0) {
					var errors_list = "<?php echo translate('cms_form_submission_errors'); ?>\n";
					for(x in submission_errors) {
						errors_list += submission_errors[x] + "\n";
					}
					alert(errors_list);
					return false;
				} else {
					return true;
				}
			}//end beforeSubmit_<?php echo $this->getPrefix() ?>()
			</script>
			<?php
			$code = ob_get_contents();
		ob_end_clean();

		return $code;

	}//end getClientSideFunction()


//--        POST-SUBMISSION PROCESSING        --//


	/**
	* Complete a submission and clean up
	*
	* Does the following actions:<ul>
	* <li>fill in final information - complete attribute and XML version of
	*   submission result</li>
	* <li>On a multi-page form with submissions logged, move the submission from
	*   incomplete to completed submissions</li>
	* <li>Or: on a single-page form, create the submission if we are
	*   logging them</li>
	* <li>Or: on a multi-page form when submissions are NOT logged, nuke the
	*   submission now we are done with using it</li></ul>
	*
	* @return void
	*/
	public function completeSubmission()
	{
		$submission_asset = $this->getSubmissionAsset();
		$all_questions = $this->getAllQuestions();

		$ok = TRUE;

		
		// if it's a multi page form, we have to revalidate all questions to prevent hacky behaviour
		if($this->attr('multi_page')) {
			foreach ($all_questions as $assetid => $question_info) {
				$question_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
				if(!$question_asset->hasValidValue()) {
				    $this->registerErrors('question_errors', Array($question_asset->id => $question_asset->failed_rules));
				    $ok = FALSE;
				}
			}
		}
		// don't bother going further, we don't want gunky file assets
		if(!$ok) return FALSE;

		foreach ($all_questions as $assetid => $question_info) {
			$type_code = $question_info['question_type_code'];
			$GLOBALS['SQ_SYSTEM']->am->includeAsset($type_code);
			if (method_exists($type_code, 'onSubmitForm')) {
				$question_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
				$question_asset->onSubmitForm($this);

				if (!empty($question_asset->failed_rules)) {
				    $this->registerErrors('question_errors', Array($question_asset->id => $question_asset->failed_rules));
				    $ok = FALSE;
				}
			}
		}

		// create the submission asset
		if ($ok) {
			if ($this->attr('log_submissions')) {
				// If public user has not saved the submission yet
				if ($GLOBALS['SQ_SYSTEM']->userPublic() && is_null($submission_asset)) {
					// Fill new submission asset with current answers
					$submission_asset = $this->createSubmission(TRUE);
					$this->submission_asset = $submission_asset;
					$this->updateSubmission($submission_asset, TRUE);
				} else {
					// It'll exist in Incomplete. Move to completed (as long
					// as the Incomplete folder actually exists...)
					$incomplete_link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_TYPE_2, 'folder', TRUE, 'incomplete_submissions');
					if ($incomplete_link) {
						$complete_link   = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_TYPE_2, 'folder', TRUE, 'submissions_folder');

						$submission_link = $GLOBALS['SQ_SYSTEM']->am->getLinkByAsset($incomplete_link['minorid'], $submission_asset->id);
						if ($submission_link) {
							// We need to force this through, for the same reason as we
							// need to force for the creation of a submission. Without
							// this, a User asset without write access to the form would
							// not be able to move it to the Completed Submissions folder
							$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);
							$GLOBALS['SQ_SYSTEM']->am->moveLink($submission_link['linkid'], $complete_link['minorid'], SQ_LINK_TYPE_3, 0);
							$GLOBALS['SQ_SYSTEM']->restoreRunLevel();
						}
					}
				}

				// Handle file uploads depending on whether create location is set
				// We then need to update the Submission to handle any changed Extra Data
				$this->_handleCompletedFileUploads();
				$this->updateSubmission($submission_asset, TRUE);

				$submission_asset->setIP($_SERVER['REMOTE_ADDR']);
				$submission_asset->setAttrValue('complete', TRUE);
				$submission_asset->setAttrValue('submitted', ts_iso8601(time()));
				$submission_asset->setAttrValue('xml', $this->getXML());
				$submission_asset->saveAttributes();
			} else {
				// Trash it if it exists
				// We need to force this through, for the same reason as we
				// need to force for the creation of a submission. Without
				// this, a User asset without write access to the form would
				// not be able to move it to the tarash and delete it permanently
				if ($submission_asset) {
					$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_SECURITY_LINK_INTEGRITY);
						$GLOBALS['SQ_SYSTEM']->am->trashAsset($submission_asset->id);
						$submission_asset->delete();
					$GLOBALS['SQ_SYSTEM']->restoreRunLevel();
				}
			}

		}//end if

		return $ok;

	}//end completeSubmission()


	/**
	* Handle file uploads for a complete submission
	*
	* Does two things, depending on whether a create location is set:
	* - Create Location: create a NOTICE link between the File asset and the submission.
	* - No Create Location: Move file upload to the submission asset's data directory.
	*
	* @return void
	*/
	protected function _handleCompletedFileUploads()
	{
		$submission_asset = $this->getSubmissionAsset();
		$file_uploads     = $this->getAllQuestionAssets('form_question_type_file_upload');

		foreach (array_keys($file_uploads) as $assetid) {
			$file_upload = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);

			$create_location = $file_upload->attr('create_location');
			if (empty($create_location)) {
				$extra_data = $file_upload->getExtraData();
				if (!empty($extra_data['temp_filesystem_path'])) {
					$filename   = $file_upload->getValue();
					$perm_path  = $submission_asset->data_path.'/attachments/'.str_replace(':', '_', $file_upload->id);

					// Move it using Fudge file_system function
					create_directory($perm_path);
					$moved = move_file($extra_data['temp_filesystem_path'], $perm_path.'/'.$filename);
					if ($moved) delete_directory(dirname($extra_data['temp_filesystem_path']));

					$extra_data = Array(
									'filesystem_path'	=> $perm_path.'/'.$filename,
								  );

					$file_upload->setExtraData($extra_data);
					$submission_asset->setExtraData($assetid, $extra_data);
				}
			} else {
				$extra_data = $file_upload->getExtraData();
				$uploaded_assetid = array_get_index($extra_data, 'existing_file_assetid', array_get_index($extra_data, 'new_file_assetid', NULL));

				if (!empty($uploaded_assetid)) {
					$uploaded_file = $GLOBALS['SQ_SYSTEM']->am->getAsset($uploaded_assetid);

					$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);
					$this->submission_asset->createLink($uploaded_file, SQ_LINK_NOTICE, 'file_submission');
					$GLOBALS['SQ_SYSTEM']->restoreRunLevel();
				}
			}
		}

	}//end _handleCompletedFileUploads()


	/**
	* function to finish off form and print thank you bodycopy
	*
	* @return boolean
	* @access public
	*/
	function finishForm()
	{
		$tyb = $this->getBodycopy('Thank You');

		ob_start();
		if($this->useAccessibleFormat())
		    $this->printSemanticSummary();
		else
		    $this->printSummary(TRUE);
		$summary_info = ob_get_contents();
		ob_end_clean();

		// check to see if the bodycopy attached to this form has any content
		if ($this->attr('use_bodycopy_thank_you') && $tyb && !$tyb->isEmpty()) {

			if ($this->attr('show_results')) {
				$keyword_list = $tyb->getKeywords();
				$replacements = Array();

				foreach ($keyword_list as $keyword) {
					$lookup_keyword = $keyword;

					// If form_info or form_summary, we need to make sure the
					// HTML version gets printed
					if (($keyword === 'form_info') || ($keyword == 'form_summary')) {
						$lookup_keyword = $keyword.'_html';
					}

					$replacement = $this->_getThankYouKeywordReplacement($lookup_keyword);
					$replacements[$keyword] = $replacement;
				}

				$tyb->setKeywordReplacements($replacements);

			}//end if

			$tyb->printBody();

		} else {
			// no bodycopy
			// print the results if we need to
			if ($this->attr('show_results')) echo $summary_info;

		}

		// Clean-up questions
		$all_questions = $this->getAllQuestions();

		foreach ($all_questions as $assetid => $question_info) {
			$type_code = $question_info['question_type_code'];
			$GLOBALS['SQ_SYSTEM']->am->includeAsset($type_code);
			if (method_exists($type_code, 'onSubmitForm')) {
				$question_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
				$question_asset->cleanUp($this);
			}
		}

		if (!is_null($this->submission_asset)) {
			$submission_path = $this->data_path.'/incomplete_attachments/s'.$this->submission_asset->id;
			if (is_dir($submission_path)){
				$dirs = list_dirs($submission_path);
				if (empty($dirs)) delete_directory($submission_path);
			}
		}

		$public_submission_path = $this->data_path.'/incomplete_attachments/'.session_id();
		if (is_dir($public_submission_path)){
			$dirs = list_dirs($public_submission_path);
			if (empty($dirs)) delete_directory($public_submission_path);
		}

		// Remove the "public submission" password, if set
		if (isset($_SESSION['SQ_FORM_'.$this->id.'_SUBMISSION_CODE'])) {
			unset($_SESSION['SQ_FORM_'.$this->id.'_SUBMISSION_CODE']);
		}

		// Clean-up submission session variable
		unset($_SESSION['SQ_FORM_'.$this->id.'_SUBMISSION']);

	}//end finishForm()


//--        SUBMISSIONS        --//


	/**
	* Get the submissions folder specified by the link value
	*
	* If called with zero arguments, points to the "Completed Submissions"
	* folder, as per previous behaviour. This SHOULD be called with an attribute
	* as much as possible, though, to avoid ambiguity.
	*
	* @param string	$link_value	The value of the link of the submissions folder to get
	*
	* @return mixed Folder|null
	* @access public
	*/
	function getSubmissionsFolder($link_value='submissions_folder')
	{
		$res = NULL;
		$link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_TYPE_2, 'folder', TRUE, $link_value);
		if (!empty($link)) {
			$res = $GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], 'folder');
		}
		return $res;

	}//end getSubmissionsFolder()


	/**
	* Create a submissions folder
	*
	* @param string	$folder_name	The name of the folder
	* @param string	$link_value		The value of the link between the form and the folder
	*
	* @return mixed Folder|null
	* @access public
	*/
	function createSubmissionsFolder($folder_name, $link_value)
	{
		$am = $GLOBALS['SQ_SYSTEM']->am;
		$ok = TRUE;

		// Find an existing folder - if it exists, just return that
		$existing_folder = $this->getSubmissionsFolder($link_value);
		if ($existing_folder) {
			return $existing_folder;
		}

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		$GLOBALS['SQ_SYSTEM']->am->includeAsset('folder');
		$folder = new Folder();
		$create_info = Array(
						'asset'			=> $this,
						'link_type'		=> SQ_LINK_TYPE_2,
						'value'			=> $link_value,
						'is_dependant'	=> 1,
						'is_exclusive'	=> 1,
					   );

		$folder->setAttrValue('name', $folder_name);
		if (!$folder->create($create_info)) {
			// Cannot create it.
			$ok = FALSE;
		}

		if ($ok) {
			// deny public read and write access by default - but may need to force it
			// because usually setting permissions requires admin access
			$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);

			$public_userid = $GLOBALS['SQ_SYSTEM']->am->getSystemAssetid('public_user');

			if (!$am->setPermission($folder->id, $public_userid, SQ_PERMISSION_WRITE, 0, TRUE, TRUE)) {
				// couldn't deny it - we can't just return the folder with write permission to a public user
				$ok = FALSE;
			}

			if (!$am->setPermission($folder->id, $public_userid, SQ_PERMISSION_READ, 0, TRUE, TRUE)) {
				$ok = FALSE;
			}

			$GLOBALS['SQ_SYSTEM']->restoreRunLevel();
		}

		// End of transaction. If things didn't work out, null the folder out
		// and rollback the transaction, otherwise push it through
		if ($ok) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		} else {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$folder = NULL;
		}

		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		return $folder;

	}//end createSubmissionsFolder()


	/**
	* creates a new submission object
	*
	* @param boolean	$create_in_completed	If true, it will be created in
	*											Completed Submissions folder - otherwise,
	*											place in Incomplete Submissions.
	*
	* @return object
	* @access public
	*/
	function &createSubmission($create_in_completed=FALSE)
	{
		$am = $GLOBALS['SQ_SYSTEM']->am;

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
		$GLOBALS['SQ_SYSTEM']->am->includeAsset('form_submission');

		if ($create_in_completed) {
			$link_value = 'submissions_folder';
		} else {
			$link_value = 'incomplete_submissions';
		}

		// create the form submission asset
		$submission_asset = new Form_Submission();
		$submissions_folder = $this->getSubmissionsFolder($link_value);
		if (is_null($submissions_folder)) {
			// If we need to create the Incomplete Submissions folder, do so now.
			// We shall not do this with the Completed Submissions folder, because
			// this should have been created when the form was created
			if (!$create_in_completed) {
				$submissions_folder = $this->createSubmissionsFolder(translate('cms_form_incomplete_submissions'), 'incomplete_submissions');
			}

			if (is_null($submissions_folder)) {
				trigger_localised_error('CMS0069', E_USER_WARNING);
				$null = NULL;
				return $null;
			}
		}
		$copy_link  = Array(
						'asset'			=> $submissions_folder,
						'link_type'		=> SQ_LINK_TYPE_3,
						'is_dependant'	=> 0,
						'is_exclusive'	=> 0,
					  );

		$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);

		if (!$submission_asset->create($copy_link)) {
			$GLOBALS['SQ_SYSTEM']->restoreRunLevel();
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return FALSE;
		}

		if ($GLOBALS['SQ_SYSTEM']->userPublic()) {
			// Set a unique passcode to the submission
			$passcode = $submission_asset->setSubmissionPasscode();
			// Store the passcode in the session
			if (!empty($passcode)) {
				$_SESSION['SQ_FORM_'.$this->id.'_SUBMISSION_CODE'] = $passcode;
			}
		}

		$submission_asset->setAttrValue('complete', FALSE);
		// initially created submission asset should have current page set to first page.
		$submission_asset->setAttrValue('current_page', 1);
		$submission_asset->saveAttributes(FALSE, $this->attr('log_submissions'));

		// The current user will receive write access to this submission,
		// (as long as one is actually logged in, ie. we are not public)
		$public_userid  = $GLOBALS['SQ_SYSTEM']->am->getSystemAssetid('public_user');
		$current_userid = $GLOBALS['SQ_SYSTEM']->currentUserid();
		if ((string)$current_userid !== (string)$public_userid) {
			if (!$am->setPermission($submission_asset->id, $current_userid, SQ_PERMISSION_WRITE, 1, TRUE, TRUE)) {
				$GLOBALS['SQ_SYSTEM']->restoreRunLevel();
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
				return FALSE;
			}
		}

		$GLOBALS['SQ_SYSTEM']->restoreRunLevel();
		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		return $submission_asset;

	}//end createSubmission()


	/**
	* Returns the current submission asset
	*
	* @param boolean $verify_passcode	Whether to verify the passcode for the
	*									public user submission
	*
	* @return object|boolean
	* @access public
	*/
	function &getSubmissionAsset($verify_passcode=TRUE)
	{
		if (!$this->submission_asset) {
			// Find the submission asset, but ensure we are getting it from _POST first
			$submission_assetid = array_get_index($_POST, 'SQ_FORM_'.$this->id.'_SUBMISSION', NULL);
			if (is_null($submission_assetid)) {
				$submission_assetid = array_get_index($_GET, 'SQ_FORM_'.$this->id.'_SUBMISSION', NULL);

			}

			// Validate the submission assetid
			if ($GLOBALS['SQ_SYSTEM']->am->getAssetInfo($submission_assetid, Array('form_submission'))) {
				// Check if asset exists first
				$this->submission_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($submission_assetid, 'form_submission');
			}

			// If we have the submission asset, make sure the user has the write access
			if (!$this->submission_asset || !$this->canEditSubmission()) {
				$this->submission_asset = NULL;
			}

			// If public user, make sure the user has the authority
			if ($verify_passcode && $this->submission_asset && $GLOBALS['SQ_SYSTEM']->userPublic()) {
				$var_name = 'SQ_FORM_'.$this->id.'_SUBMISSION_CODE';
				// Get the submission password from the POST first
				$password = array_get_index($_POST, $var_name, array_get_index($_SESSION, $var_name, ''));
				if (empty($password) || $password !== $this->submission_asset->attr('passcode')) {
					$this->submission_asset = NULL;
					if ($this->publicUserPasswordVerificationForm($submission_assetid)) {
						$pending_auth = TRUE;
						// Need to return by ref
						return $pending_auth;
					}

				} else if (isset($_POST[$var_name])) {
					// Set/Update the public passcode in the session
					$_SESSION[$var_name] = $password;
				}
			}

			// Fill up the current answers from previous parts of the submission
			if ($this->submission_asset) {
				$answers = $this->submission_asset->getAnswers();
				foreach ($answers as $assetid => $response) {
					if (is_string($response['answer'])) {
						$response['answer'] = html_entity_decode($response['answer']);
					}
					$this->current_answers[$assetid] = $response['answer'];
					$this->extra_data[$assetid]      = array_get_index($response, 'extra_data', '');
				}
				$this->repopulate();
			}
		}

		return $this->submission_asset;

	}//end getSubmissionAsset()


	/**
	* Updates the submission with the current page's processed data
	*
	* @param Form_Submission	$submission_asset	The submission asset to update
	* @param boolean		$force		Force update submission with all questions
	*
	* @return void
	*/
	public function updateSubmission(Form_Submission $submission_asset, $force=FALSE)
	{
		foreach ($this->current_answers as $assetid => $response) {

			if (!$force && !is_null($this->current_page_questions) && !in_array($assetid, $this->current_page_questions)) continue;

			$question = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);

			// Save and stick the value - but only if the question still exists
			if ($question instanceof Form_Question) {
				$question->saveValue($this);

				// If the question is sticky, stick the value then
				if ($question->attr('sticky')) {
					$question->stickValue($question->getValue());
				}

				// only re-set the answer if it has changed
				$value = $question->getValue();
				if (is_string($value)) {
					$value = htmlspecialchars($value, ENT_NOQUOTES, SQ_CONF_DEFAULT_CHARACTER_SET);
				}
				if ($value !== $submission_asset->getAnswer($assetid)) {
					$submission_asset->setAnswer($assetid, $value);
				}

				// same with Extra Data
				if ($question->getExtraData() !== $submission_asset->getExtraData($assetid)) {
					$submission_asset->setExtraData($assetid, $question->getExtraData());
				}

				$submission_asset->setSummary($assetid, $question->attr('name'), htmlspecialchars($question->getSummary(), ENT_NOQUOTES, SQ_CONF_DEFAULT_CHARACTER_SET));
			}
		}

		$submission_asset->saveAttributes(FALSE, $this->attr('log_submissions'));

		// If submission asset has metadata schema applied, broadcast the MetadataUpdate event
		$mm = $GLOBALS['SQ_SYSTEM']->getMetadataManager();
		$applied_schemas = $mm->getSchemas($submission_asset->id);
		if (!empty($applied_schemas)) {
			$em = $GLOBALS['SQ_SYSTEM']->getEventManager();
			$em->broadCastEvent($submission_asset, 'MetadataUpdate', Array('all'));
		}

	}//end updateSubmission()


	/**
	* Returns an array of asset IDs representing submissions sitting in the
	* incomplete submissions folder
	*
	* @return array
	*/
	public function getIncompleteSubmissions()
	{
		// If we are not a
		if ($GLOBALS['SQ_SYSTEM']->userPublic()) {
			return Array();
		}

		$incomplete_folder = $this->getSubmissionsFolder('incomplete_submissions');
		if (!$incomplete_folder) {
			return Array();
		}

		$sql_parts = $GLOBALS['SQ_SYSTEM']->am->generateGetChildrenQuery($incomplete_folder, 'form_submission', FALSE);
		$sql_parts['sql_array']['where'] .= ' AND (a.assetid IN (SELECT assetid FROM sq_vw_ast_perm WHERE userid = :is_userid AND permission >= :is_min_perm AND granted = :is_granted))';
		$sql_parts['bind_vars']['is_userid'] = $GLOBALS['SQ_SYSTEM']->currentUserid();
		$sql_parts['bind_vars']['is_min_perm'] = SQ_PERMISSION_WRITE;
		$sql_parts['bind_vars']['is_granted'] = '1';

		$sql = implode(' ', $sql_parts['sql_array']);

		try {
			$query = MatrixDAL::preparePdoQuery($sql);
			foreach ($sql_parts['bind_vars'] as $bind_var => $bind_value) {
				MatrixDAL::bindValueToPdo($query, $bind_var, $bind_value);
			}
			$results = array_keys(MatrixDAL::executePdoGroupedAssoc($query));
		} catch (DALException $e) {
			throw new Exception($e->getMessage());
		}

		return $results;

	}//end getIncompleteSubmissions()


	/**
	* Return the current page number
	*
	* @return string
	*/
	public function getCurrentPageNumber()
	{
		$submission_asset = $this->getSubmissionAsset();

		if ($GLOBALS['SQ_SYSTEM']->userPublic() && is_null($submission_asset)) {
			$submission_var =& $_SESSION['SQ_FORM_'.$this->id.'_SUBMISSION'];
			$current_page = array_get_index($submission_var, 'current_page', 1);
		} else {
			if ($submission_asset) {
				$current_page = $submission_asset->attr('current_page');
			} else {
				$current_page = 1;
			}
		}

		return $current_page;

	}//end getCurrentPageNumber()


	/**
	* Return the latest page number
	*
	* @return string
	*/
	public function getLatestPageNumber()
	{
		if ($GLOBALS['SQ_SYSTEM']->userPublic()) {
			$submission_var =& $_SESSION['SQ_FORM_'.$this->id.'_SUBMISSION'];
			$latest_page = array_get_index($submission_var, 'latest_page', '1');
		} else {
			$submission_asset = $this->getSubmissionAsset();
			if ($submission_asset) {
				$latest_page = $submission_asset->attr('latest_page');
			} else {
				$latest_page = '1';
			}
		}

		return $latest_page;

	}//end getLatestPageNumber()


	/**
	* Callback function to inform File Upload-type questions where to upload
	* their incomplete submissions to
	*
	* In our case, it is:
	* - incomplete_attachments/[session_id] when no incomplete submission asset exists
	*   (ie. one-page form, or anonymous submission)
	* - incomplete_attachments/s[subm. assetid] when an incomplete submission is represented
	*   by an asset
	*
	* @return string
	*/
	public function getTempFileUploadPath()
	{
		$submission_asset = $this->getSubmissionAsset();

		if ($submission_asset) {
			$path = $this->data_path.'/incomplete_attachments/s'.$submission_asset->id;
		} else {
			$path = $this->data_path.'/incomplete_attachments/'.session_id();
		}

		return $path;

	}//end getTempFileUploadPath()


//--        SUBMISSION VIA SESSION VARIABLE        --//


	/**
	* Restore an anonymous submission from a session variable
	*
	* @return void
	*/
	public function restoreFromSubmissionCookie()
	{
		$session_var_name = 'SQ_FORM_'.$this->id.'_SUBMISSION';

		if (!isset($_SESSION[$session_var_name])) {
			$_SESSION[$session_var_name] = Array();
		}

		$session_var =& $_SESSION[$session_var_name];

		foreach ($this->getAllQuestionAssets() as $question) {
			if (isset($session_var[$question->id])) {
				//$question->setValue($session_var[$question->id]['answer']);
				//$question->setExtraData($session_var[$question->id]['extra_data']);

				$this->current_answers[$question->id] = $session_var[$question->id]['answer'];
				$this->extra_data[$question->id]      = array_get_index($session_var[$question->id], 'extra_data', Array());

				$question->setValue($this->current_answers[$question->id]);
				$question->setExtraData($this->extra_data[$question->id]);
			}
		}

	}//end restoreFromSubmissionCookie()


	/**
	* Updates an anonymous submission being tracked through a session variable
	* with the current page's processed data
	*
	* @return void
	*/
	public function updateSubmissionCookie()
	{
		$session_var_name = 'SQ_FORM_'.$this->id.'_SUBMISSION';

		if (!isset($_SESSION[$session_var_name])) {
			$_SESSION[$session_var_name] = Array();
		}

		foreach ($this->current_answers as $assetid => $response) {

			if (!is_null($this->current_page_questions) && !in_array($assetid, $this->current_page_questions)) continue;

			$question = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);

			// Save and stick the value
			$question->saveValue($this);

			// If the question is sticky, stick the value then
			if ($question->attr('sticky')) {
				$question->stickValue($question->getValue());
			}

			if (isset($_SESSION[$session_var_name][$assetid])) {
				$current_answer = $_SESSION[$session_var_name][$assetid]['answer'];
			} else {
				$current_answer = NULL;
			}

			// only re-set the answer if it has changed
			if ($question->getValue() !== $current_answer) {
				$_SESSION[$session_var_name][$assetid]['answer'] = $question->getValue();
			}

			$_SESSION[$session_var_name][$assetid]['name']    = $question->attr('name');
			$_SESSION[$session_var_name][$assetid]['summary'] = $question->getSummary();
			$_SESSION[$session_var_name][$assetid]['extra_data'] = $question->getExtraData();
		}


	}//end updateSubmissionCookie()


	/**
	* Convert a submission stored in a session variable to an asset
	*/
	public function convertSubmissionCookieToAsset()
	{
		$submission_asset =& $this->createSubmission(TRUE);
		$this->updateSubmission($submission_asset, TRUE);

	}//end convertSubmissionCookieToAsset()


//--        KEYWORDS AVAILABLE        --//


	/**
	* Add valid keywords for this asset to an array of keywords when asked
	*
	* @param object	&$broadcaster	the asset that triggered the event
	* @param array	$vars			the vars that get submitted by the broadcaster
	*								we add keywords to the $vars['keywords'] array
	*
	* @return boolean
	* @access private
	*/
	function onRequestKeywords(&$broadcaster, $vars=Array())
	{
		if (!isset($vars['keywords'])) return;

		// Find out whether we've actually been sent a form section's keyword
		// request; go silent on it if it is
		$bc_parents = $GLOBALS['SQ_SYSTEM']->am->getParents($broadcaster->id);
		if (!in_array('form_section', $bc_parents)) {

			$keywords = Array();
			// search for a bodycopies folder
			$folder_link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_TYPE_2, 'folder', TRUE, 'bodycopies_folder');

			$folder = $GLOBALS['SQ_SYSTEM']->am->getAsset($folder_link['minorid'], $folder_link['minor_type_code']);

			if (is_null($folder)) return FALSE;

			// lets work out which bodycopy the WYSIWYG cell that wants our keywords
			// is in so we can return the correct keywords for the screen
			$bodycopy_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($folder->id, SQ_LINK_TYPE_2, 'bodycopy');
			$parents = $GLOBALS['SQ_SYSTEM']->am->getParents($broadcaster->id, 'bodycopy', TRUE);

			$bodycopy_code = NULL;
			foreach ($bodycopy_links as $link_info) {
				if (isset($parents[$link_info['minorid']])) {
					$bodycopy_code = $link_info['value'];
					break;
				}
			}

			if (is_null($bodycopy_code)) return;

			$keywords = @$this->_tmp['keywords'][$bodycopy_code];
			if (is_null($keywords) || !is_array($keywords)) {
				$keywords = Array();
				// note: if the value on the link is "Thank You", the method providing the keywords should be _getThankYouBodycopyKeywords
				$keyword_provider_method = '_get'.str_replace(' ','',ucfirst($bodycopy_code)).'BodycopyKeywords';

				if (method_exists($this, $keyword_provider_method)) {
					$keywords = $this->$keyword_provider_method();
					$this->_tmp['keywords'][$bodycopy_code] = $keywords;
				}
			}

			$vars['keywords'] = array_merge($vars['keywords'], $keywords);

		}

	}//end onRequestKeywords()


	/**
	* Get an array of keywords for Page Contents Bodycopy
	*
	* @return array
	* @access private
	*/
	function _getPageContentsBodycopyKeywords()
	{
		$questions = $this->getQuestionAssets();
		$sections = $this->getSections();

		$keywords['form_contents']         = translate('cms_form_contents');
		$keywords['current_page_contents'] = translate('cms_current_page_contents');
		$keywords['form_errors']           = translate('cms_form_errors');
		$keywords['form_errors_message']   = translate('cms_form_errors_message');
		$keywords['form_required_field_note']   = ucfirst(translate('cms_form_required_field_note'));

		foreach ($questions as $question) {
			$keywords['question_name_' .$question->id] = translate('cms_form_question_name',  ellipsisize($question->attr('name'), 30)).' ('.$question->id.')';
			$keywords += $this->_getCommonQuestionKeywords($question, $this);
		}

		foreach ($sections as $section) {
			$keywords['section_title_'.$section->id] = 'Section Name: '.$section->attr('name').' ('.$section->id.')';
			$keywords['section_contents_'.$section->id] = 'Section Contents: '.$section->attr('name').' ('.$section->id.')';
			$keywords += $section->getDisplayKeywords(FALSE);
		}

		// These are best at the end
		$keywords['reset_button']         = translate('cms_form_reset_button');
		$keywords['submit_button']        = translate('cms_form_submit_button');

		if ($this->getTotalPages() > 1) {
			$keywords['previous_page_button'] = translate('cms_form_previous_page_button');
		}

		if ($this->attr('require_captcha')) {
		    $keywords += $this->_getCaptchaDisplayKeywords();
		}

		if ($this->attr('use_mollom')) {
			$keywords += $this->_getMollomCaptchaDisplayKeywords();
		}

		return $keywords;

	}//end _getPageContentsBodycopyKeywords()


	/**
	* Get an array of keywords for Submission Limit Reached (bodycopy)
	*
	* @return array
	* @access private
	*/
	function _getSubmissionLimitReachedBodycopyKeywords()
	{
		$keywords['form_submissions_limit'] = translate('cms_form_submissions_limit');
		$keywords['form_submissions_count'] = translate('cms_form_submissions_count');
		$keywords['form_submissions_left']  = translate('cms_form_submissions_left');

		return $keywords;

	}//end _getSubmissionLimitReachedBodycopyKeywords()


	/**
	* Get an array of keywords for Thank You Context (bodycopy)
	*
	* @return array
	* @access private
	*/
	function _getThankYouBodycopyKeywords()
	{
		$questions = $this->getQuestions();
		$sections = $this->getSections();

		$keywords['form_summary']               = translate('cms_form_submission_summary');
		$keywords['form_summary_html']          = translate('cms_form_submission_summary').' (html)';
		$keywords['form_summary_semantic']          = translate('cms_form_submission_summary').' (semantic)';
		$keywords['form_info']                  = translate('cms_form_submission_info');
		$keywords['form_info_html']             = translate('cms_form_submission_info').' (html)';
		$keywords['form_submission_id']         = translate('cms_form_submission_id');
		$keywords['form_submission_ip_address'] = translate('cms_form_submission_ip_address');
		$keywords['form_submission_time']       = translate('cms_form_submission_time_format', translate('date_format_default'));

		foreach (get_date_format_names() as $date_name => $date_format) {
			$keywords['form_submission_time_'.$date_name] = translate('cms_form_submission_time_format', translate('date_format_'.$date_name));
		}

		foreach ($questions as $q_id => $question) {
			$q_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($this->id.':q'.$q_id);
			$q_name = $q_asset->attr('name');
			$keywords['response_'.$this->id.'_q'.$q_id]      = translate('cms_form_question_response', ellipsisize($q_name,30));
			$keywords['question_name_'.$this->id.'_q'.$q_id] = translate('cms_form_question_name', ellipsisize($q_name,30));
			$keywords['question_note_'.$this->id.'_q'.$q_id] = translate('cms_form_question_note', ellipsisize($q_name,30));
		}

		foreach ($sections as $section) {
			$keywords['section_title_'.$section->id] = translate('cms_form_section_title', $section->attr('name'));
			$keywords += $section->getResponseKeywords();
		}

		return $keywords;

	}//end _getThankYouBodycopyKeywords()


	/**
	* Get an array of keywords for the Confirmation Page Contents bodycopy
	*
	* @return array
	*/
	protected function _getConfirmationPageContentsBodycopyKeywords()
	{

		$questions = $this->getQuestionAssets();
		$sections = $this->getSections();

		$keywords = Array();

		$keywords['form_summary'] = translate('cms_form_submission_summary');
		$keywords['form_summary_html']          = translate('cms_form_submission_summary').' (html)';
		$keywords['form_summary_semantic']          = translate('cms_form_submission_summary').' (semantic)';
		$keywords['form_info']    = translate('cms_form_submission_info');

		foreach ($questions as $question) {
			$keywords += $this->_getCommonQuestionKeywords($question, $this);
		}

		foreach ($sections as $section) {
			$keywords['section_title_'.$section->id] = 'Section Name: '.$section->attr('name').' ('.$section->id.')';
			$keywords += $section->getResponseKeywords();
		}

		// These are best at the end
		$keywords['reset_button']         = translate('cms_form_reset_button');
		$keywords['submit_button']        = translate('cms_form_submit_button');
		$keywords['previous_page_button'] = translate('cms_form_previous_page_button');

		$keywords += $this->_getCaptchaDisplayKeywords();

		return $keywords;

	}//end _getConfirmationPageBodycopyKeywords()


	/**
	* Get an array of question-related keywords applicable to all contexts
	*
	* @param Form_Question	$question	The form question we getting keywords for
	* @param Asset			$parent		(Form|Form_Section) parent asset
	*
	* @return array
	*/
	protected function _getCommonQuestionKeywords(Form_Question $question, Asset $parent)
	{
		// Since we cannot assert a mixed asset argument in the formal parameter list
		// (further than "instanceof Asset"), double-check that we are being sent
		// a Form or Form Section
		assert_true(($parent instanceof Form) || ($parent instanceof Form_Section), 'Argument 2 must be a Form or Form Section asset');

		$keywords = Array();

		// Replace the shadow asset colon with an underscore
		$keyword_assetid = str_replace(':', '_', $question->id);

		if ($parent instanceof Form_Section) {
			$keywords['question_field_' .$keyword_assetid] = translate('cms_form_section_q_field',  $parent->attr('name'), ellipsisize($question->attr('name'), 30)).' ('.$question->id.')';
			$keywords['question_name_' .$keyword_assetid] = translate('cms_form_section_q_name',  $parent->attr('name'), ellipsisize($question->attr('name'), 30)).' ('.$question->id.')';
			$keywords['question_id_'   .$keyword_assetid] = translate('cms_form_section_q_id',    $parent->attr('name'), ellipsisize($question->attr('name'), 30)).' ('.$question->id.')';
			$keywords['question_note_' .$keyword_assetid] = translate('cms_form_section_q_note',  $parent->attr('name'), ellipsisize($question->attr('name'), 30)).' ('.$question->id.')';
			$keywords['question_label_'.$keyword_assetid] = translate('cms_form_section_q_label', $parent->attr('name'), ellipsisize($question->attr('name'), 30)).' ('.$question->id.')';
			$keywords['question_error_'.$keyword_assetid] = translate('cms_form_section_q_error', $parent->attr('name'), ellipsisize($question->attr('name'), 30)).' ('.$question->id.')';
		} else {
			$keywords['question_field_' .$keyword_assetid] = translate('cms_form_question_field',  ellipsisize($question->attr('name'), 30)).' ('.$question->id.')';
			$keywords['question_name_' .$keyword_assetid] = translate('cms_form_question_name',  ellipsisize($question->attr('name'), 30)).' ('.$question->id.')';
			$keywords['question_id_'   .$keyword_assetid] = translate('cms_form_question_id',    ellipsisize($question->attr('name'), 30)).' ('.$question->id.')';
			$keywords['question_note_' .$keyword_assetid] = translate('cms_form_question_note',  ellipsisize($question->attr('name'), 30)).' ('.$question->id.')';
			$keywords['question_label_'.$keyword_assetid] = translate('cms_form_question_label', ellipsisize($question->attr('name'), 30)).' ('.$question->id.')';
			$keywords['question_error_'.$keyword_assetid] = translate('cms_form_question_error', ellipsisize($question->attr('name'), 30)).' ('.$question->id.')';
		}

		return $keywords;

	}//end _getCommonQuestionKeywords()


	/**
	* Get an array of section-related keywords applicable to all contexts
	*
	* @param Form_Section	$section	The form section we are gathering keywords for
	*
	* @return array
	*/
	protected function _getCommonSectionKeywords(Form_Section $section)
	{
		$keywords = Array();

		$keywords['section_title_'.$section->id] = translate('cms_form_section_title', $section->attr('name')).' ('.$section->id.')';

		return $keywords;

	}//end _getCommonSectionKeywords()


	/**
	* Get an array of question-related keywords applicable to response contexts
	*
	* "Response contexts" include: Thank You summary pages, Confirmation page,
	*
	*
	* @param Form_Question	$question	The form question we getting keywords for
	* @param Asset			$parent		(Form|Form_Section) parent asset
	*
	* @return array
	*/
	protected function _getQuestionResponseKeywords(Form_Question $question, Asset $parent)
	{
		// Since we cannot assert a mixed asset argument in the formal parameter list
		// (further than "instanceof Asset"), double-check that we are being sent
		// a Form or Form Section
		assert_true(($parent instanceof Form) || ($parent instanceof Form_Section), 'Argument 2 must be a Form or Form Section asset');

		$keywords = Array();

		// Replace the shadow asset colon with an underscore
		$keyword_assetid = str_replace(':', '_', $question->id);

		if ($parent instanceof Form_Section) {
			$keywords['response_' .$keyword_assetid] = translate('cms_form_section_response',  $parent->attr('name'), ellipsisize($question->attr('name'), 30)).' ('.$question->id.')';
		} else {
			$keywords['response_' .$keyword_assetid] = translate('cms_form_question_response',  ellipsisize($question->attr('name'), 30)).' ('.$question->id.')';
		}

		return $keywords;

	}//end _getQuestionResponseKeywords()


	/**
	* Return CAPTCHA display keywords so they can be injected where necessary
	*
	* @return array
	*/
	protected function _getCaptchaDisplayKeywords()
	{
		$keywords = Array();

		$keywords['form_captcha']       = translate('cms_form_captcha');
		$keywords['form_captcha_image'] = translate('cms_form_captcha_image');
		$keywords['form_captcha_field'] = translate('cms_form_captcha_field');
		$keywords['form_captcha_link']  = translate('cms_form_captcha_link');
		$keywords['form_captcha_errors']  = translate('cms_form_captcha_errors');
		$keywords['form_captcha_label']       = translate('cms_form_captcha_label');

		return $keywords;

	}//end _getCaptchaDisplayKeywords()



	/**
	* Return Mollom CAPTCHA display keywords so they can be injected where necessary
	*
	* @return array
	*/
	protected function _getMollomCaptchaDisplayKeywords()
	{
		$keywords = Array();

		$keywords['form_mollom_captcha']       = translate('cms_form_mollom_captcha');
		$keywords['form_mollom_captcha_image'] = translate('cms_form_mollom_captcha_image');
		$keywords['form_mollom_captcha_audio'] = translate('cms_form_mollom_captcha_audio');
		$keywords['form_mollom_captcha_field'] = translate('cms_form_mollom_captcha_field');
		$keywords['form_mollom_captcha_regen_link']  = translate('cms_form_mollom_captcha_link');
		$keywords['form_mollom_captcha_errors']       = translate('cms_form_mollom_captcha_errors');
		$keywords['form_mollom_captcha_label']       = translate('cms_form_mollom_captcha_label');

		return $keywords;

	}//end _getMollomCaptchaDisplayKeywords()



//--        KEYWORD REPLACEMENTS        --//


	/**
	* Get a replacement for a keyword used during the display of the form
	*
	* If no suitable replacement exists, the function will return NULL instead.
	*
	* @param string	$keyword	The keyword to replace
	*
	* @return mixed string|NULL
	*/
	public function getDisplayKeywordReplacement($keyword)
	{
		$replacement = NULL;
		$keyword = parse_keyword($keyword, $modifiers);
		$is_question_keyword = preg_match('/^question\_(.*)\_(\d+[\_\:]q\d+)$/', $keyword, $question_matches);
		$is_response_keyword = preg_match('/^response\_(\d+[\_\:]q\d+)(\_raw)?$/', $keyword, $response_matches);
		$is_section_keyword  = preg_match('/^section\_(.*)\_(\d+)$/', $keyword, $section_matches);
		$is_form_keyword	 = preg_match('/^form\_(.*)$/', $keyword, $form_matches);

		if ($is_question_keyword) {
			// Sub keyword in $matches[1] - eg. 'field', 'id', 'note', 'label'
			// Question assetid in $matches[2] - but need to replace underscore
			// with colon
			list(, $sub_keyword, $assetid) = $question_matches;
			$assetid = str_replace('_', ':', $assetid);

			$question_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);

			if (!empty($question_asset)) {

				switch ($sub_keyword) {
					case 'field' :
						$replacement = $question_asset->getHtmlField();
					break;

					case 'name' :
						$replacement = $question_asset->attr('name');
					break;

					case 'id' :
						$replacement = 'q'.str_replace(':', '_', $assetid);
					break;

					case 'note' :
						$replacement = $question_asset->attr('note');
					break;

					case 'label' :
						$replacement = $question_asset->getHtmlLabel();
					break;
				    
				    	case 'error' :
						if(isset($this->question_errors[$question_asset->id])) {
						    $errors = $this->question_errors[$question_asset->id];
						    $replacement = array_pop($errors);
						}
					break;

				}//end switch
			}

		} else if ($is_section_keyword) {
			// Sub keyword in $matches[1] - eg. 'title' or 'contents'
			// Section assetid in $matches[2]
			list(, $sub_keyword, $assetid) = $section_matches;

			$section_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);

			switch ($sub_keyword) {
				case 'title' :
					$replacement = $section_asset->attr('name');
				break;

				case 'contents' :
					ob_start();
						include_once($section_asset->data_path.'/content_file.php');
					$replacement = ob_get_clean();
				break;

			}//end switch

		} else if ($is_response_keyword) {
			$raw_response = (count($response_matches) == 3);
			list(, $assetid) = $response_matches;
			$assetid = str_replace('_', ':', $assetid);

			$question_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
			if (!empty($question_asset)) {
				if ($raw_response) {
					$replacement = htmlspecialchars($question_asset->getValue(), ENT_NOQUOTES, SQ_CONF_DEFAULT_CHARACTER_SET);
				} else {
					$replacement = htmlspecialchars($question_asset->getSummary(), ENT_NOQUOTES, SQ_CONF_DEFAULT_CHARACTER_SET);
				}
			}

		} else if ($is_form_keyword) {
			list(, $sub_keyword) = $form_matches;
			switch ($sub_keyword) {
				case 'mollom_captcha':
					$replacement = Mollom::getMollomCaptchaKeywordReplacement($this->id, $this->mollom_errors);
				break;
				case 'mollom_captcha_field':
					$replacement = Mollom::getMollomCaptchaFieldKeywordReplacement($this->id);
				break;

				case 'mollom_captcha_image':
					$replacement = Mollom::getMollomCaptchaImageKeywordReplacement($this->id);
				break;

				case 'mollom_captcha_audio':
					$replacement = Mollom::getMollomCaptchaAudioKeywordReplacement($this->id);
				break;

				case 'mollom_captcha_regen_link':
					$regen_link_text = $this->attr('mollom_captcha_regen_link_text');
					if (empty($regen_link_text)) {
						$regen_link_text = translate('cms_form_mollom_regen_link_text');
					}
					$replacement = Mollom::getMollomCaptchaRegenLinkKeywordReplacement($this->id, $regen_link_text);
				break;
				
				case 'mollom_captcha_errors':
					$replacement = empty($this->mollom_errors) ? '' : array_pop($this->mollom_errors);
				break;
			    
			    	case 'mollom_captcha_label':
					$replacement = $this->attr('mollom_captcha_label_text');
				break;

				case 'submission_id':
					$submission = $this->getSubmissionAsset();
					if ($submission) {
						$replacement = $submission->id;
					}
				break;

				case 'submissions_limit':
					$replacement = $this->attr('submission_limit');
				break;

				case 'submissions_count':
					if ($this->attr('log_submissions')) {
						$replacement = $this->getSubmissionCount();
					} else {
						$replacement = '';
					}
				break;

				case 'submissions_left':
					if ($this->attr('log_submissions')) {
						$sub_limit = $this->attr('submission_limit');
						$sub_done  = $this->getSubmissionCount();
						$replacement = $sub_limit - $sub_done;
					} else {
						$replacement = '';
					}
				break;
				
				case 'required_field_note':
					$is_required_field = FALSE;
					$questions = $this->getAllQuestions();
					foreach ($questions as $question) {
					    if(isset($question['attributes']['is_required']) && $question['attributes']['is_required']) {
						$is_required_field = TRUE;
					    }
					}
					$require_note = $this->attr('required_field_note');
					if(empty($require_note))
					    $is_required_field =  FALSE;
					$replacement = $is_required_field ? $require_note : '';
				break;
			}//end switch

		}
		if (!is_null($replacement)) {
			$replace_keywords = Array(
				'assetid' 	=> $this->id,
				'call_fns'	=> Array ('getDisplayKeywordReplacement', 'getKeywordReplacement'),
			);
			apply_keyword_modifiers($replacement, $modifiers, $replace_keywords);
		}

		return $replacement;

	}//end getDisplayKeywordReplacement()


	/**
	* Current Page Contents keyword replacement
	*
	* @return string
	* @access public
	*/
	function getCurrentPageContentsKeywordReplacement()
	{
		if ($this->attr('multi_page')) {
			$current_page = $this->getCurrentPageNumber();
			return $this->getPageContents($current_page);
		} else {
			return $this->getFormContentsKeywordReplacement();
		}

	}//end getCurrentPageContentsKeywordReplacement()


	/**
	* Current Page Contents keyword replacement
	*
	* @param int	$current_page	Specify a page number to get the name of.
	*								If NULL, return current page.
	*
	* @return string
	* @access public
	*/
	function getCurrentPageNameKeywordReplacement($current_page=NULL)
	{
		if (is_null($current_page)) {
			$current_page = $this->getCurrentPageNumber();
		}
		$replacement = NULL;

		if (($current_page == $this->getTotalPages()) && $this->attr('use_confirmation_page')) {
			$replacement = $this->attr('confirmation_page_name');
		} else {
			$question_count = $this->getQuestionCount();
			if ($question_count > 0) $current_page--;

			if ($current_page == 0) {
				$replacement = $this->attr('unattached_page_name');
			} else {
				$sections = $this->getSectionLinks();
				$current_section_link = reset(array_slice($sections, $current_page - 1, 1));
				$current_section_id   = $current_section_link['minorid'];
				$current_section_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo($current_section_id);

				$replacement = $current_section_info[$current_section_id]['name'];
			}
		}

		return $replacement;

	}//end getCurrentPageContentsKeywordReplacement()


	/**
	* Page Number keyword replacement
	*
	* @return string
	* @access public
	*/
	function getPageNumberKeywordReplacement()
	{
		return $this->getCurrentPageNumber();

	}//end getPageNumberKeywordReplacement()


	/**
	* Total Pages keyword replacement
	*
	* @return string
	* @access public
	*/
	function getTotalPagesKeywordReplacement()
	{
		return $this->getTotalPages();

	}//end getTotalPagesKeywordReplacement()


	/**
	* Form Contents keyword replacement
	*
	* @return string
	* @access public
	*/
	function getFormContentsKeywordReplacement()
	{
		ob_start();
			if ($this->attr('use_confirmation_page') && ($this->getCurrentPageNumber() == $this->getTotalPages())) {
				echo $this->getConfirmationPageContents();
			} else {
				$this->includeContentFile('Standard');
			}
		return ob_get_clean();

	}//end getFormContentsKeywordReplacement()


	/**
	* Form Contents keyword replacement
	*
	* @return string
	* @access public
	*/
	function getFormContentsNoSectionsKeywordReplacement()
	{
		ob_start();
			$this->includeContentFile('Questions');
		return ob_get_clean();

	}//end getFormContentsKeywordReplacement()

	/**
	* Form Captcha keyword replacement
	*
	* @return string
	* @access public
	*/
	function getFormCaptchaKeywordReplacement()
	{
		if (!$this->attr('captcha_ttf') == '') {
			$font = $GLOBALS['SQ_SYSTEM']->am->getAsset($this->attr('captcha_ttf'));
			$file_path = $font->data_path.'/'.$font->attr('name');
		} else {
			$file_path = '';
		}
		$errors = $this->captcha_errors;
		$error = !empty($errors) ? array_pop($errors) : '';
		ob_start();
			security_key(
				$this->attr('captcha_length'),
				$this->attr('captcha_length'),
				$this->attr('captcha_zoom'),
				FALSE,
				trim($this->attr('captcha_bgcolour'), '#'),
				trim($this->attr('captcha_textcolour'), '#'),
				trim($this->attr('captcha_bordercolour'), '#'),
				$this->attr('captcha_use_colours'),
				$this->attr('captcha_usefont'),
				$file_path,
				$this->attr('captcha_font_size'),
				$this->attr('captcha_min_angle'),
				$this->attr('captcha_max_angle'),
				$this->attr('captcha_x_start'),
				$this->attr('captcha_min_distance'),
				$this->attr('captcha_max_distance'),
				$this->attr('captcha_width'),
				$this->attr('captcha_height'),
				$this->attr('captcha_arc'),
				trim($this->attr('captcha_arc_colour'), '#'),
				$this->attr('captcha_include_uppercase'),
				$this->attr('captcha_include_numbers'),
				$this->attr('provide_accessible_captcha'),
				$this->useAccessibleFormat(),
				$this->attr('captcha_label_text'),
				$error
			);
		return ob_get_clean();

	}//end getFormCaptchaKeywordReplacement()


	/**
	* Form Captcha Image keyword replacement
	*
	* @return string
	* @access public
	*/
	function getFormCaptchaImageKeywordReplacement()
	{
		if (!$this->attr('captcha_ttf') == '') {
			$font = $GLOBALS['SQ_SYSTEM']->am->getAsset($this->attr('captcha_ttf'));
			$file_path = $font->data_path.'/'.$font->attr('name');
		} else {
			$file_path = '';
		}

		ob_start();
			security_key_image_box(
				$this->attr('captcha_length'),
				$this->attr('captcha_zoom'),
				trim($this->attr('captcha_bgcolour'), '#'),
				trim($this->attr('captcha_textcolour'), '#'),
				trim($this->attr('captcha_bordercolour'), '#'),
				$this->attr('captcha_use_colours'),
				$this->attr('captcha_usefont'),
				$file_path,
				$this->attr('captcha_font_size'),
				$this->attr('captcha_min_angle'),
				$this->attr('captcha_max_angle'),
				$this->attr('captcha_x_start'),
				$this->attr('captcha_min_distance'),
				$this->attr('captcha_max_distance'),
				$this->attr('captcha_width'),
				$this->attr('captcha_height'),
				$this->attr('captcha_arc'),
				trim($this->attr('captcha_arc_colour'), '#'),
				$this->attr('captcha_include_uppercase'),
				$this->attr('captcha_include_numbers'),
				$this->attr('provide_accessible_captcha')
			);
		return ob_get_clean();

	}//end getFormCaptchaImageKeywordReplacement()


	/**
	* Form Captcha Field keyword replacement
	*
	* @return string
	* @access public
	*/
	function getFormCaptchaFieldKeywordReplacement()
	{
		ob_start();
			security_key_field($this->attr('captcha_length'));
		return ob_get_clean();

	}//end getFormCaptchaFieldKeywordReplacement()


	/**
	* Form Captcha Link keyword replacement
	*
	* @return string
	* @access public
	*/
	function getFormCaptchaLinkKeywordReplacement()
	{
		ob_start();
			security_key_image_link($this->attr('captcha_link_text'));
		return ob_get_clean();

	}//end getFormCaptchaLinkKeywordReplacement()
	
	
	/**
	* Form Captcha error keyword replacement
	*
	* @return string
	* @access public
	*/
	function getFormCaptchaErrorsKeywordReplacement()
	{
		$errors = $this->captcha_errors;
		return empty($errors) ? '' : array_pop($errors);

	}//end getFormCaptchaErrorsKeywordReplacement()

	/**
	* Form Captcha Field keyword replacement
	*
	* @return string
	* @access public
	*/
	function getFormCaptchaLabelKeywordReplacement()
	{
		return $this->attr('captcha_label_text');

	}//end getFormCaptchaLabelTextKeywordReplacement()


	/**
	* Form Errors keyword replacement
	*
	* @return string
	* @access public
	*/
	function getFormErrorsKeywordReplacement()
	{
		if (empty($this->submission_errors)) return '';

		ob_start();
			$this->printErrors();
		return ob_get_clean();

	}//end getFormErrorsKeywordReplacement()


	/**
	* Form Errors Message keyword replacement
	*
	* @return string
	* @access public
	*/
	function getFormErrorsMessageKeywordReplacement()
	{
		if (empty($this->submission_errors) && empty($this->question_errors)) return '';

		return $this->attr('submission_errors_msg');

	}//end getFormErrorsMessageKeywordReplacement()


	/**
	* Submit Button keyword replacement
	*
	* @return string
	* @access public
	*/
	function getSubmitButtonKeywordReplacement()
	{
		$current_page = $this->getCurrentPageNumber();
		$total_pages  = $this->getTotalPages();

		if ($current_page >= $total_pages) {
			$button_name = ($this->attr('submit_button')) ? $this->attr('submit_button') : translate('submit');
		} else {
			$button_name = ($this->attr('next_button')) ? $this->attr('next_button') : translate('next');
		}
		$extra = $this->attr('submit_button_extras');
		if (empty($extra))
		    $extra = 'class="sq-form-submit"';

		ob_start();
			submit_button($this->getPrefix().'_submit', $button_name, '', $extra);
		return ob_get_clean();

	}//end getSubmitButtonKeywordReplacement()


	/**
	* Save Button keyword replacement
	*
	* @return string
	* @access public
	*/
	function getSaveButtonKeywordReplacement()
	{
		$button_name = ($this->attr('save_button')) ? $this->attr('save_button') : translate('save');
		$extra = 'class="sq-form-submit"';
		ob_start();
			submit_button($this->getPrefix().'_save', $button_name, $extra);
		return ob_get_clean();

	}//end getSaveButtonKeywordReplacement()


	/**
	* Save Confirmation keyword replacement
	*
	* @return string
	* @access public
	*/
	function getSaveConfirmationKeywordReplacement()
	{
		$prefix = $this->getPrefix();
		if (isset($_POST[$prefix.'_save'])) {
			$message = ($this->attr('save_confirmation_message')) ? $this->attr('save_confirmation_message') : translate('cms_form_save_confirmation');
		} else {
			$message = '';
		}
		return $message;

	}//end getSaveConfirmationKeywordReplacement()


	/**
	* Saved incomplete submission asset info keyword replacement
	* Relevant to the Public User only
	*
	* @return string
	* @access public
	*/
	function getSubmissionAccessInfoKeywordReplacement()
	{
		$prefix = $this->getPrefix();
		if (!isset($_POST[$prefix.'_save']) || !$GLOBALS['SQ_SYSTEM']->userPublic() || !$this->attr('public_incomplete_submission')) {
			return '';
		}

		$message = $this->attr('public_auth_info_format');
		$message = $message ? $message : translate('cms_public_auth_info_format');

		if (!empty($message)) {
			$replacements = $this->getIncompleteSubmissionMessageKeywordsReplacement($message);
			$message = replace_keywords($message, $replacements);
		}

		return $message;

	}//end getSaveConfirmationKeywordReplacement()


	/**
	* "Save and Exit" Button keyword replacement
	*
	* Only display if the "Exit Page Contents" bodycopy has been enabled.
	*
	* @return string
	* @access public
	*/
	function getSaveAndExitButtonKeywordReplacement()
	{
		$bodycopy = $this->getBodycopy('Exit Page Contents');

		if ($bodycopy) {
			$button_name = ($this->attr('save_and_exit_button')) ? $this->attr('save_and_exit_button') : translate('save_and_exit');
			$extra = 'class="sq-form-submit"';
			ob_start();
				submit_button($this->getPrefix().'_save_and_exit', $button_name, $extra);
			$replacement = ob_get_clean();
		} else {
			$replacement = NULL;
		}

		return $replacement;

	}//end getSaveAndExitButtonKeywordReplacement()


	/**
	* Reset Button keyword replacement
	*
	* @return string
	* @access public
	*/
	function getResetButtonKeywordReplacement()
	{
		$button_name = ($this->attr('reset_button')) ? $this->attr('reset_button') : translate('reset');
		$extra = $this->attr('reset_button_extras');
		if (empty($extra))
		    $extra = 'class="sq-form-reset"';
		ob_start();
			reset_button($this->getPrefix().'_reset', $button_name, '', $extra);
		return ob_get_clean();

	}//end getResetButtonKeywordReplacement()


	/**
	* Previous Page Button keyword replacement
	*
	* @return string
	* @access public
	*/
	function getPreviousPageButtonKeywordReplacement()
	{
		// Only display the previous page keyword if we are beyond the first page
		$current_page = $this->getCurrentPageNumber();

		if ($current_page > 1) {
			$button_name = ($this->attr('previous_button')) ? $this->attr('previous_button') : translate('previous_page');
			$extra = 'class="sq-form-submit"';
			ob_start();
				submit_button($this->getPrefix().'_previous_page', $button_name, $extra);
			$replacement = ob_get_clean();
		} else {
			$replacement = '';
		}

		return $replacement;

	}//end getPreviousPageButtonKeywordReplacement()


	/**
	* Get a keyword replacement for Page List
	*
	* @return string
	*/
	public function getPageListKeywordReplacement()
	{
		ob_start();

		$submission_asset = $this->getSubmissionAsset();
		$current_page = $this->getCurrentPageNumber();
		$latest_page = $this->getLatestPageNumber();

		$total_pages = $this->getTotalPages();

		$page_list_format = $this->attr('page_list_format');
		$keywords = extract_keywords($page_list_format);

		?>
		<ul class="sq-form-page-list">
			<?php
			for ($page_number = 1; $page_number <= $total_pages; $page_number++) {
				// Get this page's very own keyword replacements
				$page_entry = $page_list_format;
				$replacements = Array();

				$replacements['page_number'] = htmlspecialchars($page_number, ENT_NOQUOTES, SQ_CONF_DEFAULT_CHARACTER_SET);
				$replacements['total_pages'] = htmlspecialchars($total_pages, ENT_NOQUOTES, SQ_CONF_DEFAULT_CHARACTER_SET);
				$replacements['page_name'] = htmlspecialchars($this->getCurrentPageNameKeywordReplacement($page_number), ENT_NOQUOTES, SQ_CONF_DEFAULT_CHARACTER_SET);

				replace_keywords($page_entry, $replacements);

				$page_has_link = ($page_number <= $latest_page) && ($page_number != $current_page);

				$style_classes = Array();
				if ($page_number == $current_page) {
					$style_classes[] = 'sq-form-current-page-number';
				} else if ($page_number > $latest_page) {
					$style_classes[] = 'sq-form-unseen-page-number';
				} else if ($page_number > $current_page) {
					$style_classes[] = 'sq-form-later-page-number';
				} else {
					$style_classes[] = 'sq-form-prior-page-number';
				}

				if ($page_number == $latest_page) {
					$style_classes[] = 'sq-form-last-seen-page-number';
				}
			?>
				<li<?php echo !empty($style_classes) ? ' class="'.implode(' ', $style_classes).'"' : '' ?>><?php
					if ($page_has_link) {
						?><a href="<?php echo $this->getURL() ?>?SQ_FORM_<?php echo $this->id ?>_PAGE=<?php echo $page_number ?><?php
						if ($submission_asset) {
							echo '&SQ_FORM_'.$this->id.'_SUBMISSION='.$submission_asset->id;
						}
						?>"><?php
					}

					echo $page_entry;

					if ($page_has_link) {
						?></a><?php
					}
				?></li>
			<?php }//end for
		?></ul>
		<?php
		return ob_get_clean();

	}//end getPageListKeywordReplacement()


	/**
	* Get appropriate keywords for Thank You bodycopy and emails
	*
	* This can be extended further down the line. For instance, Form_Email (the
	* "standard" custom form asset in Matrix) adds referrer-related keywords.
	*
	* @param string	$keyword	keyword to replace
	*
	* @return string
	*/
	public function _getThankYouKeywordReplacement($keyword)
	{
		$full_keyword = $keyword;
		$replacement = '%'.$keyword.'%';
		$prefix = $this->getPrefix();
		$keyword = parse_keyword($keyword, $modifiers);

		// Easier to use if-else if rather than switch, because some of these
		// will be regex matches

		if ($keyword === 'form_info_html') {
			// Form info keyword - HTML email
			// %form_info_html%

			$replacement = $this->getFormInfo(TRUE);

		} else if ($keyword === 'form_info_text') {
			// Form info keyword - text email
			// %form_info_text%

			$replacement = $this->getFormInfo(FALSE);

		} else if ($keyword === 'form_summary_html') {
			// Form summary keyword - HTML email
			// %form_summary_html%

			ob_start();
				$this->printSummary(TRUE);
				$replacement = ob_get_contents();
			ob_end_clean();

		} else if ($keyword === 'form_summary_semantic') {
			// Form semantic summary - intended for HTML ONLY

			ob_start();
				$this->printSemanticSummary();
				$replacement = ob_get_contents();
			ob_end_clean();

		} else if ($keyword === 'form_summary_text') {
			// Form summary keyword
			// %form_summary_text%

			ob_start();
				$this->printSummary(FALSE);
				$replacement = ob_get_contents();
			ob_end_clean();

		} else if ($keyword === 'form_submission_id') {
			// Asset ID of the form submission
			// %form_submission_id%

			if (!is_null($this->submission_asset)) {
				$replacement = $this->submission_asset->id;
			}

		} else if (preg_match('|^form_submission_time(_?.*)|', $keyword, $matches)) {
			// Submission time - can use date formats - if not recognised as
			// a valid shorthand date format, it's assumed to be a date() format
			// (just like date keywords elsewhere)
			// %form_submission_time_rfc2822%

			if (empty($matches[1])) {
				$date_format = 'Y-m-d H:i:s';
			} else {
				$date_format = substr($matches[1], 1);
			}

			if (!is_null($this->submission_asset)) {
				$date_formats_list = get_date_formats();
				if (array_key_exists($date_format, $date_formats_list)) {
					$replacement = format_date($this->submission_asset->created, $date_format);
				} else {
					$replacement = date($date_format, $this->submission_asset->created);
				}
			}

		} else if ($keyword === 'form_submission_ip_address') {
			// IP address of the submitting user
			// %form_submission_ip_address%

			// TODO: Should we handle proxy-forwarded IP addresses
			//       (X-Forwarded-For header)?
			$ip_address = $_SERVER['REMOTE_ADDR'];
			$replacement = $ip_address;

		} else if (preg_match('|^form_asset_(.*)|', $keyword, $matches)) {
			// form PAGE asset keyword
			// eg. %form_asset_assetid%

			$parent_link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_TYPE_2, 'page_custom_form', FALSE, NULL, 'minor');
			$parent_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($parent_link['majorid']);

			$form_keyword = 'asset_'.$matches[1];
			$asset_replacement = $parent_asset->getKeywordReplacement($form_keyword);
			if ($asset_replacement !== "%$form_keyword%") {
				$replacement = $asset_replacement;
			}

		} else {
			// questions that depend on knowing what questions and sections we have
			// We will cache them though, so we don't load them up all the time

			if (empty($this->_tmp['keyword_cache'])) {
				$this->_tmp['keyword_cache']['questions'] = $this->getQuestions();
				$this->_tmp['keyword_cache']['sections' ] = $this->getSections();
			}

			$questions =& $this->_tmp['keyword_cache']['questions'];
			$sections  =& $this->_tmp['keyword_cache']['sections'];

			if (preg_match('|^section_title_(.*)|', $keyword, $matches)) {
				// Section title keyword
				// eg. %section_title_123%

				$section_assetid = $matches[1];
				$section_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($section_assetid, 'form_section');
				if (!empty($section_asset)) {
					$replacement = $section_asset->attr('name');
				}

			} else if (preg_match('|^question_([^_]*)_([^_]*_)?q(.*)|', $keyword, $matches)) {
				// Question name/note keywords, eg.
				// %question_note_123_q4% ... note for q.4 from form/section
				//                            with ID 123
				// %question_name_q4% ....... name of q.4 from the form

				$question_keyword = $matches[1];
				if (empty($matches[2])) {
					// One asset ID
					$section_assetid = $this->id;
					$question_assetid = $matches[3];
				} else {
					$section_assetid = str_replace('_', '', $matches[2]);
					$question_assetid = $matches[3];
				}

				$question_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($section_assetid.':q'.$question_assetid);
				if (!empty($question_asset)) {
					switch ($question_keyword) {
						case 'name':
							$replacement = $question_asset->attr('name');
						break;

						case 'note':
							$replacement = $question_asset->attr('note');
						break;
					}
				}

			} else if (preg_match('|^response_([0-9]*_)?q([0-9]*)?(_raw)?|', $keyword, $matches)) {
				// Question response keywords, eg.
				// %response_123_q4% ... question 4 from form/section ID 123
				// %response_q4% ....... question 4 from the form

				if (empty($matches[1])) {
					// One asset ID
					$section_assetid = $this->id;
					$question_assetid = $matches[2];
				} else {
					$section_assetid = str_replace('_', '', $matches[1]);
					$question_assetid = $matches[2];
				}

				$raw = (array_get_index($matches, 3, '') == '_raw');

				$question_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($section_assetid.':q'.$question_assetid);
				if (!empty($question_asset)) {
					if ($raw) {
						$value = $question_asset->getValue();
						if (is_string($question_asset->getValue())) {
							$replacement = htmlspecialchars($question_asset->getValue(), ENT_NOQUOTES, SQ_CONF_DEFAULT_CHARACTER_SET);
						} else {
							$replacement = '';
						}
					} else {
						$replacement = htmlspecialchars($question_asset->getSummary(), ENT_NOQUOTES, SQ_CONF_DEFAULT_CHARACTER_SET);
					}
				}

			}

		}//end else - keyword list

		// if replacement not found yet, see if it is not context specific
		if ($replacement == '%'.$keyword.'%') {
			$replacement = $this->getKeywordReplacement($keyword);
		}

		// If replacement still not found, leave it to be replaced later
		if ($replacement != '%'.$full_keyword.'%') {
			$replace_keywords = Array(
							'assetid' 	=> $this->id,
							'call_fns'	=> Array ('_getThankYouKeywordReplacement', 'getKeywordReplacement'),
						);
			apply_keyword_modifiers($replacement, $modifiers, $replace_keywords);
		}

		return $replacement;

	}//end _getThankYouKeywordReplacement()


	/**
	* Perform any section submission actions that are valid and active
	* for current page in the form
	*
	* @param int	$page_number  	Current page number
	*
	* @access public
	* @return boolean
	*/
	function performFormSectionActions($page_number)
	{
		$success = TRUE;
		if ($this->attr('multi_page')) {

			$section_links = $this->getSectionLinks();

			// Get current section
			$section_link = reset(array_slice($section_links, $page_number - 1, 1));
			if (isset($section_link['minorid'])) {
				$section = $GLOBALS['SQ_SYSTEM']->am->getAsset($section_link['minorid']);

				$actions = $section->attr('actions');
				foreach ($actions as $action) {
					$action_type = $action['type_code'];
					$GLOBALS['SQ_SYSTEM']->am->includeAsset($action_type);

					if ($action['active'] && call_user_func(Array($action_type, 'isValid'), $section, $action['settings'])) {
						$success = call_user_func(Array($action_type, 'execute'), $section, $this, $action['settings']);
						if (!$success) {
							break;
						}
					}//end if
				}//end foreach

				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($section);
			}
		}

		return $success;

	}//end performFormSectionActions()


	/**
	* Perform any operations that needs to be done for selected section
	* submission actions, after all the individual form step actions are
	* executed and the form is submitted
	*
	* @access public
	* @return void
	*/
	function _finaliseStepsActions()
	{
		if ($this->attr('multi_page')) {

			$section_links = $this->getSectionLinks();
			foreach($section_links as $section_link) {
				if (!isset($section_link['minorid'])) continue;
				$section = $GLOBALS['SQ_SYSTEM']->am->getAsset($section_link['minorid']);
				$actions = $section->attr('actions');

				foreach ($actions as $action) {
					$action_type = $action['type_code'];
					$GLOBALS['SQ_SYSTEM']->am->includeAsset($action_type);
					call_user_func(Array($action_type, 'finalise'), $this);
				}//end foreach

				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($section);
			}//end foreach
		}//end if

	}//end _finaliseStepsActions()


	/**
	* Gets the (completed) submission count for this form
	*
	* @access public
	* @return boolean/int
	*/
	function getSubmissionCount()
	{
		if (!$this->attr('log_submissions')) return FALSE;

		$submission_folder = $this->getSubmissionsFolder();
		if (is_null($submission_folder)) return FALSE;

		$submissions = $GLOBALS['SQ_SYSTEM']->am->getChildren($submission_folder->id, 'form_submission', TRUE, NULL, NULL, NULL, TRUE, 1, 1);

		return count($submissions);

	}//end getSubmissionCount()


	/**
	* Prints the Submission limit reached bodycopy
	*
	* @param array	$replacements	a list of additional replacements that
	*								subclasses can use to insert additional
	*								keywords
	*
	* @return void
	* @access public
	*/
	function printSubmissionLimitReachedBodycopy($replacements=Array())
	{
		$bodycopy = $this->getBodycopy('Submission Limit Reached');

		// check to see if the bodycopy attached to this form has any content
		if ($bodycopy && !$bodycopy->isEmpty()) {
			$keyword_list = $bodycopy->getKeywords();

			if (!empty($keyword_list)) {
				foreach ($keyword_list as $keyword) {
					// Question and section-related keywords are found here
					$replacement = $this->getDisplayKeywordReplacement($keyword);

					if (is_null($replacement)) {
						// If not picked up by first function, this will pick up
						// replacements placed in separate functions - CAPTCHA,
						// errors, for instance
						$replacement = $this->getKeywordReplacement($keyword);
					}

					if (!is_null($replacement)) {
						// We have something from one of the above
						$replacements[$keyword] = $replacement;
					}
				}
			}

			$bodycopy->setKeywordReplacements($replacements);
			$bodycopy->printBody();
		}

	}//end printSubmissionLimitReachedBodycopy()

	
	/**
	* add submission errors
	* @param string	$type submission_errors / captcha_errors / question_errors / mollom_errors
	* @param array	$error the error message array							
	*
	* @return null
	* @access public
	*/
	function registerErrors($type, $error)
	{
	    if(!$this->useAccessibleFormat()) {
		 // question errors are special, take out the question id
		 if($type === 'question_errors') {
		     foreach ($error as $e) {
			 $generic_error = array_values($e);
		     }
		 }
		 else {
		     $generic_error = $error;
		 }
		$generic_type = 'submission_errors';
		if (is_null($this->$generic_type)) $this->$generic_type = Array();
		$this->$generic_type = array_merge($this->$generic_type, $generic_error);
	    }

	    if (is_null($this->$type)) $this->$type = Array();
	    $this->$type = array_merge($this->$type, $error);

	}//end registerErrors()
	
	
	/**
	* Check if  form's using accessible format
	*								
	*
	* @return boolean
	* @access public
	*/
	function useAccessibleFormat()
	{
		return $this->attr('use_accessible_format');
	}//end useAccessibleFormat()
	
	
	/**
	* Get required field mark attribute
	*								
	*
	* @return boolean
	* @access public
	*/
	function getRequiredFieldMark()
	{
		if(!$this->useAccessibleFormat()) return '';
		return ' '.$this->attr('required_field_mark');
	}//end getRequiredFieldMark()


	/**
	* If the current user is allowed to edit the submission
	*
	* @return boolean
	* @access public
	*/
	function canEditSubmission()
	{
		if (is_null($this->submission_asset)) {
			return FALSE;
		}

		// Public user will not have write access on submission assets
		// However if it was created by them then they might be allowed to edit it (feature #6306)
		$public_userid = $GLOBALS['SQ_SYSTEM']->am->getSystemAssetid('public_user');
		if ($GLOBALS['SQ_SYSTEM']->userPublic() && $public_userid == $this->submission_asset->created_userid) {
			return TRUE;
		}

		return $this->submission_asset->writeAccess();

	}//end canEditSubmission()


	/**
	* Get the special keywords used in in the incomplete form
	* submission notification message/email
	*
	* @param $text
	* @param $submission_assetid
	*
	* @return string|null
	* @access public
	*/
	function getIncompleteSubmissionMessageKeywordsReplacement($text, $submission_assetid='')
	{
		$keywords = extract_keywords($text);
		$replacements = Array();

		if (empty($keywords)) {
			return $replacements;
		}

		if (empty($submission_assetid)) {
			$submission_asset = $this->getSubmissionAsset(FALSE);
			if (is_null($submission_asset)) {
				return $replacements;
			}
			$submission_assetid = $submission_asset->id;
		} else {
			$submission_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($submission_assetid);
		}
		if (is_null($submission_asset)) {
			return $replacements;
		}

		// Generate a unique hash string for the submission access url. Relevant to Public User only
		$public_userid = $GLOBALS['SQ_SYSTEM']->am->getSystemAssetid('public_user');
		$url_hash = $submission_asset->created_userid == $public_userid ? '&SQ_FORM_'.$this->id.'_ACCESSID='.md5($submission_asset->created.$submission_asset->id) : '';

		$parent_assetids = $GLOBALS['SQ_SYSTEM']->am->getDependantParents($this->id);
		$parent_assetid = reset($parent_assetids);
		$parent_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($parent_assetid);
		$form_url = $parent_asset->getUrl();

		foreach($keywords as $full_keyword) {
			$replacement = NULL;

			$keyword = parse_keyword($full_keyword, $modifiers);

			switch ($keyword) {
				case 'form_submission_password':
					// Relevant to the Public Submissions only
					$replacement = $submission_asset->attr('passcode');
				break;

				case 'form_submission_url_id' :
					$replacement = $submission_assetid;
				break;

				case 'form_total_pages' :
					$replacement = $this->getTotalPages();
				break;

				case 'return_to_form_url_current' :
					$replacement = $form_url.'?SQ_FORM_'.$this->id.'_SUBMISSION='.$submission_assetid.'&SQ_FORM_'.$this->id.'_PAGE='.$submission_asset->attr('current_page').$url_hash;
				break;

				case 'return_to_form_url_latest' :
					$replacement = $form_url.'?SQ_FORM_'.$this->id.'_SUBMISSION='.$submission_assetid.'&SQ_FORM_'.$this->id.'_PAGE='.$submission_asset->attr('latest_page').$url_hash;
				break;

				default:
					if (substr($keyword, 0, 11) === 'form_asset_') {
						// Get the 'asset_*' keyword from the form PARENT page
						$sub_keyword = substr($keyword, 11);
						$replacement = $parent_asset->getKeywordReplacement('asset_'.$sub_keyword);
					} else if (substr($keyword, 0, 11) === 'user_asset_') {
						// Get the 'asset_*' keyword from the created user
						$sub_keyword = substr($keyword, 11);
						$user = $GLOBALS['SQ_SYSTEM']->am->getAsset($created_userid);
						if(empty($user)) continue;
						$replacement = $user->getKeywordReplacement('asset_'.$sub_keyword);
					} else if (substr($keyword, 0, 17) === 'submission_asset_') {
						// Get the 'asset_*' keyword from the submission
						$sub_keyword = substr($keyword, 17);
						$replacement = $submission_asset->getKeywordReplacement('asset_'.$sub_keyword);
					}
			}//end switch on keyword

			if (!is_null($replacement)) {
				apply_keyword_modifiers($replacement, $modifiers);
				$replacements[$full_keyword] = $replacement;
			}
		}//end foreach

		$GLOBALS['SQ_SYSTEM']->am->forgetAsset($parent_asset);

		return $replacements;

	}//end getIncompleteSubmissionMessageKeywordsReplacement()


	/**
	* Outputs a simple form with a text field and submit button to authorise 
	* the Public User to allow retrival of the previously saved submission
	*
	* @param int	$submission_assetid
	*
	* @return boolean
	* @access public
	*/
	function publicUserPasswordVerificationForm($submission_assetid)
	{
		if (!$GLOBALS['SQ_SYSTEM']->userPublic() || empty($submission_assetid)) {
			return FALSE;
		}

		// Check the public user has the authorised URL
		$access_code = array_get_index($_GET, 'SQ_FORM_'.$this->id.'_ACCESSID', '');
		if (empty($access_code)) {
			return FALSE;
		}
		$submission_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($submission_assetid, 'form_submission');
		if ($access_code != md5($submission_asset->created.$submission_asset->id)) {
			return FALSE;
		}

		$prefix = $this->getPrefix();
		$form_format = $this->attr('public_auth_form_format');
		$form_format = trim($form_format) ? $form_format : translate('cms_public_auth_form_format');
		$submit_button_text = $this->attr('public_auth_submit_button_text');
		$submit_button_text = trim($submit_button_text) ? $submit_button_text : translate('cms_public_auth_submit_button_text');

		ob_start();
			password_box('SQ_FORM_'.$this->id.'_SUBMISSION_CODE', '', 17);
		$password_text_input = ob_get_clean();

		ob_start();
			submit_button($prefix.'_public_auth', $submit_button_text);
		$submit_button_input = ob_get_clean();

		// Incorrect password message
		$error_message = $this->attr('public_auth_error_message');
		if (empty($error_message)) {
			$error_message = translate('cms_public_auth_error_message');
		}
		// If form was previously submitted and we are still here means - the password is incorrect
		$error_message = isset($_POST[$prefix.'_public_auth']) ? $error_message : '';

		// Replacements for the password verification form
		$replacements = $this->getIncompleteSubmissionMessageKeywordsReplacement($form_format, $submission_assetid);
		$replacements += Array(
							'public_form_error_message' => $error_message,
							'public_form_password_input' => $password_text_input,
							'public_form_submit_button' => $submit_button_input,
						);
		$form_format = replace_keywords($form_format, $replacements);

		// Print the password validation form
		$action_url = $_SERVER['REQUEST_URI'];
		echo '<form method="post" action="'.$action_url.'">';
		echo $form_format;
		echo '</form>';

		return TRUE;

	}//end publicUserPasswordVerificationForm()


	/**
	* Sends the submission info email notification to the Public User
	*
	* @return void
	* @access public
	*/
	function sendPublicUserSubmissionInfoEmail()
	{
		$public_userid = $GLOBALS['SQ_SYSTEM']->am->getSystemAssetid('public_user');
		if (is_null($this->submission_asset) || $this->submission_asset->created_userid != $public_userid) {
			return;
		}

		// Authorised Public User email address, if any
		$public_email_qid = $this->attr('public_auth_email_q');
		$public_reg_email = $public_email_qid ? $this->submission_asset->getAnswer($public_email_qid) : '';
		$email_format = $this->getAttribute('public_email_notification_format');
		$old_value = $new_value = unserialize($email_format->value);
		if (!empty($public_reg_email)) {
			$new_value['to'][] = $public_reg_email;
		}

		if (empty($new_value['to']) && empty($new_value['to_assetids'])) {
			return;
		}

		$keywords = $email_format->getKeywords();
		$keywords_str = !empty($keywords) ? ('%'.implode('%%', $keywords).'%') : '';
		$replacements = $this->getIncompleteSubmissionMessageKeywordsReplacement($keywords_str);

		$email_format->setValue(serialize($new_value));
		$email_format->sendMail($replacements);
		$email_format->setValue(serialize($old_value));

	}//end sendPublicUserSubmissionInfoEmail()


}//end class
?>
