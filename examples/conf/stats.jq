{
  date_time: .date_time,
  ues: [
    .nodes[0]
      | .mac.slicing.ueSliceConfig.ues + .mac.stats.ueStats + .pdcp.ueStats
        | group_by(.rnti)
          | map(add)[]
  ]
}
