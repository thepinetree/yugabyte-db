---
title: Create a cluster
linkTitle: Create a cluster
description: Create clusters in Yugabyte Cloud.
headcontent:
image: /images/section_icons/deploy/enterprise.png
aliases:
  - /latest/deploy/yugabyte-cloud/create-clusters/
  - /latest/yugabyte-cloud/create-clusters/
menu:
  latest:
    identifier: create-clusters
    parent: cloud-basics
    weight: 30
isTocNested: true
showAsideToc: true
---

As a fully managed YugabyteDB-as-a-service, Yugabyte Cloud makes it easy for you to create YugabyteDB clusters.

To create a cluster, on the **Clusters** page, click **Add Cluster** to start the **Create Cluster** wizard.

{{< note title="Note" >}}

Before creating a paid cluster, create a billing profile and add a payment method. Refer to [Manage your billing profile and payment method](../../cloud-admin/cloud-billing-profile/).

If you want to use dedicated VPCs for network isolation and security, contact [Yugabyte Support](https://support.yugabyte.com/hc/en-us/requests/new?ticket_form_id=360003113431) to set up VPC Peering with your client VPC. This has to be done before you create your cluster.

{{< /note >}}

## Create Cluster Wizard

The **Create Cluster** wizard has the following three pages:

1. [Select Cluster Type](#select-cluster-type)
1. [Cluster Settings](#cluster-settings)
1. [Database Admin Credentials](#database-admin-credentials)

### Select Cluster Type

Use the **Free** cluster to get started with YugabyteDB. Although not suitable for production workloads, the free cluster includes enough resources to start exploring the core features available for developing applications with YugabyteDB, including:

- single node
- depending on the cloud provider, up to 2 vCPUs and 10 GB of storage
- limit of one per account

**Paid** clusters support multi-node and highly available deployments and include the following features:

- no limit on cluster size - choose any cluster size based on your use case
- horizontal and vertical scaling - add or remove nodes and add storage to suit your production loads
- VPC peering support
- automated and on-demand backups
- create as many as you need

If you have not already provided payment information, you will need to add it before you can create a paid cluster.

![Add Cluster Wizard - Select Type](/images/yb-cloud/cloud-addcluster1-type.png)

Select **Yugabyte Cloud Free** or **Yugabyte Cloud** and click **Next** to display the **Cluster Settings** page.

### Cluster Settings

![Add Cluster Wizard - Cluster Settings](/images/yb-cloud/cloud-addcluster-paid2.png)

Set the following options:

- **Provider**: Choose a cloud provider - AWS or GCP. (For Azure, contact Yugabyte Support.) 
- **Cluster Name**: Enter a name for the cluster.
- **Region**: Choose the Region where the cluster will be located.

If you are creating a **Paid** cluster, set the following additional options:

- **Fault Tolerance** determines how resilient the cluster is to node and cloud zone failues: 

  - **None** - single node, with no replication or resiliency. Recommended for development and testing only.
  - **Node Level** - a minimum of 3 nodes deployed in a single availability zone with a [replication factor](../../../architecture/docdb-replication/replication/) (RF) of 3. YugabyteDB can continue to do reads and writes even in case of a node failure, but this configuration is not resilient to cloud availability zone outages. For horizontal scaling, you can scale nodes in increments of 1.
  - **Availability Zone Level** - a minimum of 3 nodes spread across multiple availability zones with a RF of 3. YugabyteDB can continue to do reads and writes even in case of a cloud availability zone failure. This configuration provides the maximum protection for a data center failure. Recommended for production deployments. For horizontal scaling, nodes are scaled in increments of 3.

- **VPC Peer**: If you want to use dedicated VPCs for network isolation and security, contact Yugabyte Support to set up VPC Peering with your client VPC. Note this has to be done before you create your cluster.

- **Cluster Configuration**:

  - Nodes - enter the number of nodes for the cluster. Node and Availability zone level clusters have a minimum of 3 nodes; Availability zone level clusters increment by 3.
  - vCPU/Node - enter the number of virtual CPUs per node.
  - Disk size/Node - enter the disk size per node in GB.

  Paid clusters support both horizontal and vertical scaling; you can change the cluster configuration after the cluster is created using the **Edit Configuration** settings. Refer to [Configure clusters](../configure-clusters#infrastructure). <!--For cluster configuration costs, refer to [Cluster costs](../cloud-admin/cloud-billing-costs/).-->

### Database Admin Credentials

The admin credentials are required to connect to the YugabyteDB database that is installed on the cluster. (You can add additional users once the cluster is provisioned.)

![Add Cluster Wizard - Admin Settings](/images/yb-cloud/cloud-addcluster-admin.png)

You can use the default credentials generated by Yugabyte Cloud, or add your own.

Copy the credentials, select **I have copied the admin credentials** and click **Create Cluster**.

{{< warning title="Important" >}}

Save your credentials in a safe place. If you lose these credentials you will not be able to use the database.

{{< /warning >}}

## Viewing the cluster

After you complete the wizard, the [**Clusters**](../../cloud-clusters/) page appears with the provisioning of your new cluster in progress. 

![Cluster being provisioned](/images/yb-cloud/cloud-cluster-provisioning.png)

Once the cluster is ready, the cluster [Overview](../../cloud-clusters/overview) is displayed.

You now have a fully configured YugabyteDB cluster provisioned in Yugabyte Cloud with the credentials you specified.

## Next steps

- [Assign IP allow lists](../add-connections/)
- [Add database users](../add-users/)
- [Connect to your cluster](../connect-to-clusters)
- [Create a database](../create-databases)
- [Connect an application](../connect-application)
