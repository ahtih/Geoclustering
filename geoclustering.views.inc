<?php
// $Id

	// Include join handler file because Views somehow does not do it
require_once dirname(__FILE__) . '/geoclustering_handler_join_dummy.inc';

/**
 * Implementation of hook_views_data().
 */
function geoclustering_views_data()
{
	$data=array();

	$clusters_table=array(

	'table' => array(/*!!! 'group' => t('Geoclustering'),	Geo+Views do not like this */
			'base' => array('field' => 'cluster_id',
							'title' => t('Geographic cluster'),
							'help' => t('Clusters of nodes ' .
									'maintained by Geoclustering module.')),

				// Fake 'join' entry, just to make it possible to add
				//  nr_of_nodes field to a Node view. Our dummy join
				//  handler makes sure that no SQL is generated for the join,
				//  and geoclustering_views_query_alter() removes the field
				//  as well

			'join' => array('node' => array(
						'left_field' => 'nid',
						'field' => 'min_nid',
						'handler' => 'geoclustering_handler_join_dummy')),
			),

	'params_hash' => array('title' => t('Clustering id'),
		'help' => t('Clustering ID defining a particular set of clusters'),
		'filter' => array('handler' => 'views_handler_filter_equality')),
			//!!! should eliminate 'Is not equal to' operator here
			//!!! change this to tree name filter/argument handler instead

	'level' => array('title' => t('Tree level'),
		'help' => t('Tree level in a particular Clustering'),
		'filter' => array('handler' => 'views_handler_filter_equality')),
			//!!! should eliminate 'Is not equal to' operator here

	'idx' => array('title' => t('Cluster idx'),
		'help' => t('Cluster idx'),
		'field' => array('dummy' => 'dummy')),

	//!!! change lat/lon fields to Geo field

	'latitude' => array('title' => t('Cluster centerpoint latitude'),
		'help' => t('Latitude of centerpoint of cluster, in WGS84'),
		'field' => array('dummy' => 'dummy'),
		'filter' => array('handler' => 'views_handler_filter_numeric')),

	'longitude' => array('title' => t('Cluster centerpoint longitude'),
		'help' => t('Longitude of centerpoint of cluster, in WGS84'),
		'field' => array('dummy' => 'dummy'),
		'filter' => array('handler' => 'views_handler_filter_numeric')),

	'nr_of_nodes' => array('title' => t('Number of nodes'),
		'help' => t('Number of nodes in cluster'),
		'field' => array('dummy' => 'dummy'),
		'filter' => array('handler' => 'views_handler_filter_numeric')),
	);

	foreach (array('volume','trustworthiness') as $field) {		//!!! load from ctools/DB, union over all clusterings
		$clusters_table['summed_field_' . $field]=array(
			'title' => t('Summed @fieldname',array('@fieldname' => $field)),
			'help' => t('Sum of @fieldname fields in a cluster',
											array('@fieldname' => $field)),
			'field' => array(
					'handler' => 'geoclustering_handler_field_summed_fields',
					'field' => 'summed_fields'
					),
			);
		}

	$data['geoclustering_clusters']=$clusters_table;

	$data['node']['geoclustering_level']=array(
		'group' => t('Geoclustering'),
		'title' => t('Geoclustering level'),
		'help' => t('Pseudo-argument to take the view data from ' .
					'Geoclustering clusters instead of Nodes, if needed'),
		'argument' => array(
			'handler' => 'geoclustering_handler_argument_level',
			'numeric' => TRUE,
			),
		);


		/*************************************/
		/*****                           *****/
		/***** Add join for {node} table *****/
		/*****                           *****/
		/*************************************/

	$data['node']['table']['join']['geoclustering_clusters']=array(
		'left_field' => 'min_nid',
		'field' => 'nid',
		'extra' => array(array(	'table' => 'geoclustering_clusters',
								'field' => 'nr_of_nodes',
								'value' => 1,
								'numeric' => TRUE)),
		);

		/***********************************************/
		/*****                                     *****/
		/***** Add join for {node_revisions} table *****/
		/*****                                     *****/
		/***********************************************/

	$data['node_revisions']['table']['join']['geoclustering_clusters']=array(
		'left_table' => 'node',
		'left_field' => 'vid',
		'field' => 'vid',
		);

		/****************************************/
		/*****                              *****/
		/***** Add joins for all CCK tables *****/
		/*****                              *****/
		/****************************************/

	foreach (content_fields() as $cck_field) {
		$db_info=content_database_info($cck_field);
		$data[$db_info['table']]['table']['join']['geoclustering_clusters']=
										array(	'left_table' => 'node',
												'left_field' => 'vid',
												'field' => 'vid',
												);
		}

	return $data;
	}

function geoclustering_views_handlers()
{
	return array(
		'handlers' => array(
			'geoclustering_handler_field_summed_fields' => array(
						'parent' => 'views_handler_field_numeric'),
			'geoclustering_handler_argument_level' => array(
						'parent' => 'views_handler_argument'),
			'geoclustering_handler_join_dummy' => array(
						'parent' => 'views_join'),
			));
	}
